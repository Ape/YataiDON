#include "entry.h"
#include "../libs/input.h"
#include "../libs/scores.h"
#include "../libs/global_data.h"
#include "../libs/config.h"
#include "../libs/network.h"
#include <algorithm>

void EntryScreen::on_screen_start() {
    Screen::on_screen_start();
    side = 1;
    is_2p = false;
    box_manager = std::make_unique<BoxManager>(global_data.entry_join_pending);
    if (global_data.config->network.auto_login) {
        state = EntryState::SELECT_SIDE;
    } else {
        state = EntryState::WAITING;
    }

    const bool online = global_data.config->network.online_play;
    pending_card_hex_.clear();
    used_cards_.clear();
    local_login_ = false;
    login_ready_ = false;
    if (online) network.probe_online();

    // Card scanned on title (case 5) or auto login (case 3): join on first update
    if (global_data.card_reader_card_valid && !global_data.card_reader_card_id_hex.empty()) {
        pending_card_hex_ = global_data.card_reader_card_id_hex;
        used_cards_.push_back(pending_card_hex_);
    }
    global_data.card_reader_card_valid = false;
    global_data.card_reader_card_id_hex.clear();

#ifdef NETWORK_ENABLED
    if (online) {
        card_reader_ = std::make_unique<card_reader::CardReader>();
        if (card_reader_->initialize(global_data.config->card_reader.port, global_data.config->card_reader.baudrate)) {
            card_reader_->start_polling();
            card_reader_->set_led_color(255, 255, 255);
            last_card_poll_ms_ = get_current_ms();
            card_reader_initialized_ = true;
        } else {
            card_reader_.reset();
        }
    }
#endif

    // Preview only: local DB, no network until someone actually logs in
    {
        auto pd = scores_manager.get_player_data(global_data.config->network.access_code_1);
        nameplate = Nameplate(
            pd ? pd->username : "", pd ? pd->title : "",
            PlayerNum::ALL,
            pd ? pd->dan : -1, pd ? pd->gold : false, pd ? pd->rainbow : false, pd ? pd->title_bg : 0);
    }

    timer = std::make_unique<Timer>(60, get_current_ms(), [this]() {
        if (box_manager->is_costume_box()) {
            box_manager->open_costume_menu(players[0] ? players[0]->player_num : PlayerNum::P1);
        } else {
            if (!box_manager->selection_allowed()) box_manager->move_left();
            box_manager->select_box();
        }
    });

    lua_entry = std::make_unique<EntryScript>();
    lua_entry->start_side_select();
    reload_preview_chara(PlayerNum::P1);
    announce_played = false;
    players.clear();
    players.resize(2);
    audio.play_sound("bgm", VolumePreset::MUSIC);

    // Online: auto_login shows side select right away, otherwise wait for Enter/Don
    if (online && !global_data.config->network.auto_login && !global_data.entry_join_pending)
        state = EntryState::WAITING;

    if (global_data.entry_join_pending) {
        global_data.entry_join_pending = false;
        start_second_player_join();
    }
}

// Local row (modifiers, dan, ...) overlaid with whatever the server knows.
PlayerData EntryScreen::sync_profile_from_server(const std::string& access_code) {
    PlayerData data = scores_manager.get_player_data(access_code).value_or(PlayerData{});
    data.player_id = access_code;

    network.fetch_chara_colors(access_code, data.chara_color_1, data.chara_color_2, data.chara_color_3);

    std::string s;
    if (network.fetch_username(access_code, s) && !s.empty()) data.username = s;
    if (data.username.empty()) data.username = "Player " + access_code.substr(0, 8);
    if (network.fetch_title(access_code, s) && !s.empty()) data.title = s;
    network.fetch_title_bg(access_code, data.title_bg);
    network.fetch_costume(access_code, data.chara_head_index, data.chara_body_index,
                          data.chara_cos_index, data.chara_is_costume);
    return data;
}

// Resolve + load the account for the next free login slot (1st login -> access_code_1, 2nd -> access_code_2).
// card_hex empty: key/auto login. Returns false if login impossible.
bool EntryScreen::login_slot(const std::string& card_hex) {
    const int slot = players[0] ? 1 : 0;
    auto& cfg = global_data.config->network;
    std::string& code = slot == 0 ? cfg.access_code_1 : cfg.access_code_2;
    std::string& sm_id = slot == 0 ? scores_manager.player_1 : scores_manager.player_2;
    PlayerData& sm_data = slot == 0 ? scores_manager.player_1_data : scores_manager.player_2_data;
    const bool net = cfg.online_play && network.is_online() && !local_login_;

    // Don after this slot was migrated to a server code: use a fresh local "0"/"1" account instead.
    // In-memory only (restored on title), so the migrated code stays in the config for Enter logins.
    if (cfg.online_play && local_login_ && code != "0" && code != "1") {
        if (!global_data.card_override[slot]) global_data.card_prev_code[slot] = code;
        global_data.card_override[slot] = true;
        code = slot == 0 ? "0" : "1";
    }

    PlayerData pd;
    if (!net) {
        // Case 1 / offline: scores.db keyed by config code. Cards can't resolve without server (case 4).
        if (!card_hex.empty()) return false;
        pd = scores_manager.get_player_data(code).value_or(PlayerData{});
        pd.player_id = code;
    } else {
        if (!card_hex.empty()) {
            std::string new_code = network.register_user("Player " + card_hex.substr(0, 8), card_hex);
            if (new_code.empty()) return false;
            if (!global_data.card_override[slot]) global_data.card_prev_code[slot] = code;
            global_data.card_override[slot] = true;
            code = new_code;
        } else if (code.empty() || code == "0" || code == "1") {
            // Cases 2/3, never registered: get a code from server, replace the placeholder
            std::string old_code = code;
            std::string name = sm_data.username.empty() ? "Player" : sm_data.username;
            std::string new_code = network.register_user(name);
            if (!new_code.empty()) {
                scores_manager.migrate_player_id(old_code, new_code);
                code = new_code;
                save_config(*global_data.config);
            }
        }
        pd = sync_profile_from_server(code);

        if (network.check_import_requested(code)) {
            scores_manager.export_to_hiroba(code, code);
            network.clear_import_flag(code);
        }
        if (cfg.sync_scores) scores_manager.sync_from_server(code, code);
    }

    sm_id = code;
    sm_data = pd;
    scores_manager.save_player_data(pd);
    if (slot == 0) {
        nameplate = Nameplate(pd.username, pd.title, PlayerNum::ALL, pd.dan, pd.gold, pd.rainbow, pd.title_bg);
    }
    return true;
}

// Card scan: log in, then show side select. The player still picks the side.
void EntryScreen::join_with_card(const std::string& card_hex) {
    if (login_ready_ || !login_slot(card_hex)) return;
    login_ready_ = true;
    audio.play_sound("don", VolumePreset::SOUND);
    if (players[0]) {
        // Second player: same screen as pressing Don for the free seat
        const PlayerNum free_seat = (players[0]->player_num == PlayerNum::P1) ? PlayerNum::P2 : PlayerNum::P1;
        const PlayerData& pd = scores_manager.player_2_data;
        nameplate = Nameplate(pd.username, pd.title, PlayerNum::ALL, pd.dan, pd.gold, pd.rainbow, pd.title_bg);
        reload_preview_chara(free_seat);
    }
    state = EntryState::SELECT_SIDE;
    side = 1;
    lua_entry->restart_side_select();
}

void EntryScreen::start_second_player_join() {
    const PlayerNum first  = global_data.entry_joined_seat;
    const PlayerNum second = (first == PlayerNum::P1) ? PlayerNum::P2 : PlayerNum::P1;

    side = (first == PlayerNum::P1) ? 0 : 2;
    global_data.player_num = first;
    players[0] = std::make_unique<EntryPlayer>(first, side, box_manager.get());
    players[0]->start_animations();

    const int second_side = (second == PlayerNum::P1) ? 0 : 2;
    global_data.player_num = second;
    players[1] = std::make_unique<EntryPlayer>(second, second_side, box_manager.get());
    players[1]->start_animations();
    audio.play_sound("cloud", VolumePreset::SOUND);
    audio.play_sound("entry_start_" + std::to_string((int)second) + "p", VolumePreset::VOICE);

    global_data.player_num = PlayerNum::P1;
    is_2p = true;
    side = 1;
    state = EntryState::SELECT_MODE;
    spdlog::info("[2P join] ENTRY resumed: {}P kept (first_login {}P), {}P entered, mode select",
                 (int)first, (int)global_data.first_login_player, (int)second);
}

void EntryScreen::reload_preview_chara(PlayerNum player_num) {
    std::string player_id = get_player_id(player_num);
    auto pd = scores_manager.get_player_data(player_id);
    chara = make_chara_from_player_data(pd ? &*pd : nullptr);
    if (pd) {
        chara->set_don_colors(pd->chara_color_1, pd->chara_color_2, pd->chara_color_3);
        chara->apply_face(pd->chara_face_index);
    } else {
        chara->set_don_colors(chara_default_color_1(player_num), chara_default_color_2(player_num), {249, 240, 225, 255});
    }
}

Screens EntryScreen::on_screen_end(Screens next_screen) {
    audio.stop_sound("bgm");

    // Clean up card reader
#ifdef NETWORK_ENABLED
    if (card_reader_) {
        card_reader_->stop_polling();
        card_reader_->led_reset();
        card_reader_.reset();
        card_reader_initialized_ = false;
    }
#endif

    return Screen::on_screen_end(next_screen);
}

bool EntryScreen::arcade_credit() const {
    return tex.options[SCO::ENTRY_CREDIT_ARCADE];
}

bool EntryScreen::seat_joined(PlayerNum player_num) const {
    for (auto& player : players) {
        if (player && player->player_num == player_num) return true;
    }
    return false;
}

void EntryScreen::join_player(PlayerNum player_num, bool do_login) {
    if (do_login && !login_ready_ && !login_slot("")) return;
    login_ready_ = false;
    side = (player_num == PlayerNum::P1) ? 0 : 2;
    global_data.player_num = player_num;

    if (players[0]) {
        players[1] = std::make_unique<EntryPlayer>(global_data.player_num, side, box_manager.get());
        players[1]->start_animations();
        global_data.player_num = PlayerNum::P1;
        is_2p = true;
    } else {
        global_data.first_login_player = global_data.player_num;
        players[0] = std::make_unique<EntryPlayer>(global_data.player_num, side, box_manager.get());
        players[0]->start_animations();
        is_2p = false;
    }
    audio.play_sound("cloud", VolumePreset::SOUND);
    audio.play_sound("entry_start_" + std::to_string((int)player_num) + "p", VolumePreset::VOICE);
    if (state == EntryState::SELECT_SIDE) lua_entry->decide_side_select(side);
    state = EntryState::SELECT_MODE;
    audio.play_sound("don", VolumePreset::SOUND);
}

std::optional<Screens> EntryScreen::handle_input() {
    if (state == EntryState::WAITING) {
        const bool enter = ray::IsKeyPressed(ray::KEY_ENTER);
        const bool don = is_l_don_pressed() || is_r_don_pressed();
        if (!enter && !don) return std::nullopt;
        local_login_ = !enter;  // Enter: register/fetch online; Don: local db only
        if (login_slot("")) {
            login_ready_ = true;
            audio.play_sound("don", VolumePreset::SOUND);
            state = EntryState::SELECT_SIDE;
            lua_entry->restart_side_select();
        }
        return std::nullopt;
    }
    // Enter again with one player in: online login (registers placeholder "1") for the free seat
    if (global_data.config->network.online_play && ray::IsKeyPressed(ray::KEY_ENTER) &&
        (state == EntryState::SELECT_SIDE || state == EntryState::SELECT_MODE) &&
        seat_joined(PlayerNum::P1) != seat_joined(PlayerNum::P2)) {
        local_login_ = false;
        join_player(seat_joined(PlayerNum::P1) ? PlayerNum::P2 : PlayerNum::P1);
        return std::nullopt;
    }
    if (arcade_credit() && state == EntryState::SELECT_SIDE) {
        if (!seat_joined(PlayerNum::P1) &&
            (is_l_don_pressed(PlayerNum::P1) || is_r_don_pressed(PlayerNum::P1))) {
            join_player(PlayerNum::P1);
        } else if (!seat_joined(PlayerNum::P2) &&
                   (is_l_don_pressed(PlayerNum::P2) || is_r_don_pressed(PlayerNum::P2))) {
            join_player(PlayerNum::P2);
        }
        return std::nullopt;
    }
    if (state == EntryState::SELECT_SIDE) {
        if (is_l_don_pressed() || is_r_don_pressed()) {
            if (side == 1) {
                if (players[0]) {
                    audio.play_sound("don", VolumePreset::SOUND);
                    state = EntryState::SELECT_MODE;
                    return std::nullopt;
                }
                return on_screen_end(Screens::TITLE);
            }
            join_player((side == 0) ? PlayerNum::P1 : PlayerNum::P2);
        }
        if (is_l_kat_pressed()) {
            audio.play_sound("kat", VolumePreset::SOUND);
            if (players[0] && players[0]->player_num == PlayerNum::P1)
                side = 1;
            else if (players[0] && players[0]->player_num == PlayerNum::P2)
                side = 0;
            else
                side = std::max(0, side - 1);
        }
        if (is_r_kat_pressed()) {
            audio.play_sound("kat", VolumePreset::SOUND);
            if (players[0] && players[0]->player_num == PlayerNum::P1)
                side = 2;
            else if (players[0] && players[0]->player_num == PlayerNum::P2)
                side = 1;
            else
                side = std::min(2, side + 1);
        }
    } else if (state == EntryState::SELECT_COSTUME) {
        for (auto& player : players) {
            if (player) player->handle_input();
        }
    } else if (state == EntryState::SELECT_MODE) {
        if (arcade_credit()) {
            if (!seat_joined(PlayerNum::P1) &&
                (is_l_don_pressed(PlayerNum::P1) || is_r_don_pressed(PlayerNum::P1))) {
                join_player(PlayerNum::P1);
                return std::nullopt;
            }
            if (!seat_joined(PlayerNum::P2) &&
                (is_l_don_pressed(PlayerNum::P2) || is_r_don_pressed(PlayerNum::P2))) {
                join_player(PlayerNum::P2);
                return std::nullopt;
            }
            if (!mode_select_ready()) return std::nullopt;
            for (auto& player : players) {
                if (player) player->handle_input();
            }
            return std::nullopt;
        }
        if (!mode_select_ready()) return std::nullopt;
        for (auto& player : players) {
            if (player) player->handle_input();
        }
        if (players[0] && players[0]->player_num == PlayerNum::P1 && (is_l_don_pressed(PlayerNum::P2) || is_r_don_pressed(PlayerNum::P2))) {
            audio.play_sound("don", VolumePreset::SOUND);
            state = EntryState::SELECT_SIDE;
            {
                auto pd = scores_manager.get_player_data(global_data.config->network.access_code_2);
                nameplate = Nameplate(
                    pd ? pd->username : "", pd ? pd->title : "",
                    PlayerNum::ALL,
                    pd ? pd->dan : -1, pd ? pd->gold : false, pd ? pd->rainbow : false, pd ? pd->title_bg : 0);
            }
            lua_entry->restart_side_select();
            side = 1;
            reload_preview_chara(PlayerNum::P2);
        } else if (players[0] && players[0]->player_num == PlayerNum::P2 && (is_l_don_pressed(PlayerNum::P1) || is_r_don_pressed(PlayerNum::P1))) {
            audio.play_sound("don", VolumePreset::SOUND);
            state = EntryState::SELECT_SIDE;
            {
                auto pd = scores_manager.get_player_data(global_data.config->network.access_code_1);
                nameplate = Nameplate(
                    pd ? pd->username : "", pd ? pd->title : "",
                    PlayerNum::ALL,
                    pd ? pd->dan : -1, pd ? pd->gold : false, pd ? pd->rainbow : false, pd ? pd->title_bg : 0);
            }
            lua_entry->restart_side_select();
            side = 1;
            reload_preview_chara(PlayerNum::P1);
        }
    }
    return std::nullopt;
}

std::optional<Screens> EntryScreen::update() {
    Screen::update();
    double current_time = get_current_ms();

    if (!players.empty() && !pending_card_hex_.empty()) {
        std::string hex = std::move(pending_card_hex_);
        pending_card_hex_.clear();
        join_with_card(hex);
    }

#ifdef NETWORK_ENABLED
    // Case 6: any card while a seat is free (P1 first, then P2)
    if (card_reader_ && card_reader_->is_polling() && !login_ready_ && state != EntryState::SELECT_COSTUME &&
        !(seat_joined(PlayerNum::P1) && seat_joined(PlayerNum::P2))) {
        if (current_time - last_card_poll_ms_ >= global_data.config->card_reader.poll_interval_ms) {
            last_card_poll_ms_ = current_time;
            if (card_reader_->poll_once()) {
                const auto& ci = card_reader_->get_card_info();
                if (ci.valid && !ci.card_id_hex.empty() &&
                    std::find(used_cards_.begin(), used_cards_.end(), ci.card_id_hex) == used_cards_.end()) {
                    used_cards_.push_back(ci.card_id_hex);
                    card_reader_->set_led_color(0, 255, 0);
                    join_with_card(ci.card_id_hex);
                }
            }
        }
    }
#endif

    allnet_indicator.update(current_time);
    entry_overlay.update(current_time);
    lua_entry->update(current_time);
    box_manager->update(current_time, is_2p);
    if (state != EntryState::WAITING && !(arcade_credit() && state == EntryState::SELECT_SIDE)) {
        timer->update(current_time);
    }
    nameplate.update(current_time);
    chara->update(current_time);
    for (auto& player : players) {
        if (player) player->update(current_time);
    }
    if (box_manager->costume_menu_open) {
        box_manager->costume_menu_open = false;
        state = EntryState::SELECT_COSTUME;
        for (auto& player : players) {
            if (player && player->player_num == box_manager->opening_player)
                player->open_costume_menu();
        }
    }
    if (state == EntryState::SELECT_COSTUME) {
        bool any_open = false;
        for (auto& player : players) {
            if (player && player->costume_menu.has_value()) { any_open = true; break; }
        }
        if (!any_open) state = EntryState::SELECT_MODE;
    }
    if (box_manager->is_finished()) {
        return on_screen_end(box_manager->selected_box());
    }
    for (auto& player : players) {
        if (player && player->is_cloud_animation_finished() &&
            !audio.is_sound_playing("entry_start_" + std::to_string((int)global_data.player_num) + "p") &&
            !announce_played) {
            audio.play_sound("select_mode", VolumePreset::VOICE);
            announce_played = true;
        }
    }
    return handle_input();
}

void EntryScreen::draw_background() {
    lua_entry->draw_background();
}

void EntryScreen::draw_side_select(float fade) {
    auto& skin = tex.skin_config;
    lua_entry->draw_side_select();

    const bool show_preview = !arcade_credit() || seat_joined(PlayerNum::P1) || seat_joined(PlayerNum::P2);
    if (show_preview) {
        chara->draw(tex.skin_config[SC::CHARA_ENTRY].x, tex.skin_config[SC::CHARA_ENTRY].y);
    }
    lua_entry->draw_side_select_buttons(side);
    if (show_preview) {
        nameplate.draw(skin[SC::NAMEPLATE_ENTRY].x, skin[SC::NAMEPLATE_ENTRY].y, fade);
    }
}

void EntryScreen::draw_player_drum() {
    for (auto& player : players) {
        if (player) player->draw_drum();
    }
}

bool EntryScreen::mode_select_ready() {
    for (auto& player : players) {
        if (player && !player->is_cloud_animation_finished()) return false;
    }
    return true;
}

void EntryScreen::draw_mode_select() {
    if (!mode_select_ready()) return;
    box_manager->draw();
}

void EntryScreen::draw() {
    draw_background();
    draw_player_drum();

    if (state == EntryState::WAITING) {
        // nothing: side select hidden
    } else if (state == EntryState::SELECT_SIDE) {
        draw_side_select(lua_entry->get_side_select_fade());
    } else if (state == EntryState::SELECT_MODE) {
        draw_mode_select();
    } else if (state == EntryState::SELECT_COSTUME) {
        for (auto& player : players) {
            if (player) player->draw_costume_menu();
        }
    }

    bool p1_joined = (players[0] && players[0]->player_num == PlayerNum::P1) ||
                     (players[1] && players[1]->player_num == PlayerNum::P1);
    bool p2_joined = (players[0] && players[0]->player_num == PlayerNum::P2) ||
                     (players[1] && players[1]->player_num == PlayerNum::P2);
    lua_entry->draw_footer(p1_joined, p2_joined);

    for (auto& player : players) {
        if (player) {
            player->draw_nameplate_and_indicator(player->get_nameplate_fadein());
        }
    }

    lua_entry->draw_player_entry();

    if (box_manager->is_finished()) {
        ray::DrawRectangle(0, 0, tex.screen_width, tex.screen_height, ray::BLACK);
    }

    timer->draw();
    entry_overlay.draw(0, tex.skin_config[SC::ENTRY_OVERLAY_ENTRY].y);
    coin_overlay.draw();
    allnet_indicator.draw();
}
