#include "modifier.h"

#include "../../libs/audio.h"

const std::array<std::string, 5> ModifierSelector::BASE_MOD_NAMES = {"auto", "speed", "display", "inverse", "random"};

bool ModifierSelector::get_bool(int mod_index) {
    const std::string& n = mod_names[mod_index];
    if (n == "auto") {
        return player->modifier_auto;
    }
    if (n == "display") {
        return player->modifier_display;
    }
    if (n == "inverse") {
        return player->modifier_inverse;
    }
    if (n == "skip") {
        return player->modifier_skip;
    }
    return false;
}

void ModifierSelector::set_bool(int mod_index, bool value) {
    const std::string& n = mod_names[mod_index];
    if (n == "auto") {
        player->modifier_auto = value;
    } else if (n == "display") {
        player->modifier_display = value;
    } else if (n == "inverse") {
        player->modifier_inverse = value;
    } else if (n == "skip") {
        player->modifier_skip = value;
    }
}

ModifierSelector::ModifierSelector(PlayerNum player_num, PlayerData* player) : player_num(player_num), player(player) {
    static int next_instance_id = 0;
    instance_id = ++next_instance_id;
    current_mod_index = 0;
    is_confirmed = false;
    is_finished = false;
    direction = -1;
    language = global_data.config->general.language;

    mod_names.assign(BASE_MOD_NAMES.begin(), BASE_MOD_NAMES.end());
    if (tex.options[SCO::OPTION_SKIP_ROW]) {
        mod_names.push_back("skip");
    }
    if (tex.options[SCO::OPTION_NEIRO_ROW]) {
        load_neiro_names();
        mod_names.push_back("neiro");
    }

    blue_arrow_fade = (FadeAnimation*)tex.get_animation(29, true);
    blue_arrow_move = (MoveAnimation*)tex.get_animation(30, true);
    move = (MoveAnimation*)tex.get_animation(28, true);
    move_out = tex.has_animation(43) ? (MoveAnimation*)tex.get_animation(43, true) : nullptr;
    move->start();
    move_sideways = (MoveAnimation*)tex.get_animation(31, true);
    fade_sideways = (FadeAnimation*)tex.get_animation(32, true);

    audio.play_sound("voice_options_" + std::to_string((int)player_num) + "p", VolumePreset::VOICE);
}

ModifierSelector::~ModifierSelector() {
    if (!neiro_preview.empty()) {
        audio.unload_sound(neiro_preview);
    }
}

void ModifierSelector::load_neiro_names() {
    std::filesystem::path neiro_list_path =
        std::filesystem::path("Skins") / global_data.config->paths.skin / "Sounds" / "hit_sounds" / "neiro_list.txt";

    std::ifstream neiro_list(neiro_list_path);
    if (!neiro_list.is_open()) {
        spdlog::error("Failed to open neiro_list.txt");
    } else {
        std::string line;
        while (std::getline(neiro_list, line)) {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            if (!line.empty()) {
                neiro_names.push_back(line);
            }
        }
    }
    neiro_names.push_back(tex.skin_text("modifier_text_muon", language, "無音"));

    neiro_index = player->neiro_index;
    if (neiro_index == -1) {
        neiro_index = (int)neiro_names.size() - 1;
    }
    neiro_index = std::clamp(neiro_index, 0, (int)neiro_names.size() - 1);
    player->neiro_index = (neiro_index == (int)neiro_names.size() - 1) ? -1 : neiro_index;
}

void ModifierSelector::step_neiro(int dir) {
    int count = (int)neiro_names.size();
    neiro_index = ((neiro_index + dir) % count + count) % count;
    player->neiro_index = (neiro_index == count - 1) ? -1 : neiro_index;

    if (!neiro_preview.empty()) {
        audio.unload_sound(neiro_preview);
    }
    neiro_preview.clear();
    if (neiro_index == count - 1) {
        return;
    }

    std::filesystem::path base = std::filesystem::path("Skins") / global_data.config->paths.skin / "Sounds" /
                                 "hit_sounds" / std::to_string(neiro_index);
    neiro_preview = audio.load_sound(base / (neiro_index == 0 ? "don.wav" : "don.ogg"), "hit_sound");
    if (!neiro_preview.empty()) {
        audio.play_sound(neiro_preview, VolumePreset::HITSOUND);
    }
}

void ModifierSelector::update(double current_ms) {
    move->update(current_ms);
    blue_arrow_fade->update(current_ms);
    blue_arrow_move->update(current_ms);
    move_sideways->update(current_ms);
    fade_sideways->update(current_ms);
    is_finished = is_confirmed && move->is_finished;
}

void ModifierSelector::confirm() {
    if (is_confirmed) {
        return;
    }
    current_mod_index++;
    if (current_mod_index == (int)mod_names.size()) {
        is_confirmed = true;
        if (move_out) {
            move = move_out;
            move->start();
        } else {
            move->restart();
        }
    }
}

void ModifierSelector::start_text_animation(int dir) {
    move_sideways->start();
    fade_sideways->start();
    direction = dir;
    change_seq++;
}

static int speed_step(int value, int dir) {
    if (dir > 0) {
        if (value >= 40) {
            return 1;
        }
        if (value >= 30) {
            return 40;
        }
        if (value >= 20) {
            return 30;
        }
        return value + 1;
    }
    if (value <= 1) {
        return 40;
    }
    if (value <= 20) {
        return value - 1;
    }
    if (value <= 30) {
        return 20;
    }
    return 30;
}

void ModifierSelector::left() {
    if (is_confirmed) {
        return;
    }
    const std::string& mod_name = mod_names[current_mod_index];
    if (row_greyed(mod_name)) {
        return;
    }

    if (mod_name == "speed") {
        player->modifier_speed = speed_step(player->modifier_speed, -1);
        start_text_animation(1);
    } else if (mod_name == "neiro") {
        step_neiro(-1);
        start_text_animation(1);
    } else if (mod_name == "random") {
        player->modifier_random = (player->modifier_random + 2) % 3;
        start_text_animation(1);
    } else {
        set_bool(current_mod_index, !get_bool(current_mod_index));
        start_text_animation(1);
    }
}

void ModifierSelector::right() {
    if (is_confirmed) {
        return;
    }
    const std::string& mod_name = mod_names[current_mod_index];
    if (row_greyed(mod_name)) {
        return;
    }

    if (mod_name == "speed") {
        player->modifier_speed = speed_step(player->modifier_speed, +1);
        start_text_animation(-1);
    } else if (mod_name == "neiro") {
        step_neiro(1);
        start_text_animation(-1);
    } else if (mod_name == "random") {
        player->modifier_random = (player->modifier_random + 1) % 3;
        start_text_animation(-1);
    } else {
        set_bool(current_mod_index, !get_bool(current_mod_index));
        start_text_animation(-1);
    }
}

std::string ModifierSelector::row_label(const std::string& n) const {
    if (n == "auto") {
        return font_manager.get_text_label("modifier_name_auto");
    }
    if (n == "speed") {
        return font_manager.get_text_label("modifier_name_speed");
    }
    if (n == "display") {
        return font_manager.get_text_label("modifier_name_display");
    }
    if (n == "inverse") {
        return font_manager.get_text_label("modifier_name_inverse");
    }
    if (n == "random") {
        return font_manager.get_text_label("modifier_name_random");
    }
    if (n == "skip") {
        return font_manager.get_text_label("modifier_name_skip");
    }
    if (n == "neiro") {
        return font_manager.get_text_label("modifier_name_neiro");
    }
    return std::string();
}

bool ModifierSelector::row_greyed(const std::string& name) const {
    return name == "skip" && global_data.player_num == PlayerNum::TWO_PLAYER;
}

std::vector<ModifierSelector::ModRow> ModifierSelector::lua_rows() {
    std::vector<ModRow> rows;
    rows.reserve(mod_names.size());
    for (int i = 0; i < (int)mod_names.size(); i++) {
        const std::string& n = mod_names[i];
        ModRow r;
        r.name = n;
        r.label = row_label(n);
        r.greyed = row_greyed(n);
        if (n == "speed") {
            r.value = std::format("{:.1f}", player->modifier_speed / 10.0f);
            r.state = player->modifier_speed;
            r.changed = player->modifier_speed != 10;
        } else if (n == "neiro") {
            r.value = neiro_names.empty() ? std::string() : neiro_names[neiro_index];
            r.state = neiro_index;
            r.changed = player->neiro_index != 0;
        } else if (n == "random") {
            if (player->modifier_random == 1) {
                r.value = font_manager.get_text_label("modifier_text_kimagure");
            } else if (player->modifier_random == 2) {
                r.value = font_manager.get_text_label("modifier_text_detarame");
            } else {
                r.value = font_manager.get_text_label("modifier_text_false");
            }
            r.state = player->modifier_random;
            r.changed = player->modifier_random != 0;
        } else {
            bool v = get_bool(i);
            r.state = v ? 1 : 0;
            r.value = v ? font_manager.get_text_label("modifier_text_true")
                        : font_manager.get_text_label("modifier_text_false");
            r.changed = v;
        }
        r.enabled = !is_confirmed && i >= current_mod_index;
        rows.push_back(std::move(r));
    }
    return rows;
}
