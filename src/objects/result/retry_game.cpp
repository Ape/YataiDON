#include "retry_game.h"

RetryGameScene::RetryGameScene(int player_num, int norma) {
    if (!load("RetryGame", "retry_game", player_num, norma)) {
        return;
    }
    fn_update = lua_object["update"];
    fn_draw_back = lua_object["draw_back"];
    fn_draw_front = lua_object["draw_front"];
    fn_chara_pos = lua_object["chara_pos"];
    fn_phase = lua_object["phase"];
    fn_hit = lua_object["hit"];
    fn_is_finished = lua_object["is_finished"];
    fn_is_success = lua_object["is_success"];
}

void RetryGameScene::update(double current_ms) {
    call(fn_update, "RetryGame:update", current_ms);
}
void RetryGameScene::draw_back() {
    call(fn_draw_back, "RetryGame:draw_back");
}
void RetryGameScene::draw_front() {
    call(fn_draw_front, "RetryGame:draw_front");
}

std::string RetryGameScene::phase() {
    return call_r<std::string>(fn_phase, "RetryGame:phase").value_or("wait");
}

std::optional<std::array<float, 3>> RetryGameScene::chara_pos() {
    auto pos = call_r<sol::table>(fn_chara_pos, "RetryGame:chara_pos");
    if (!pos) {
        return std::nullopt;
    }
    return std::array<float, 3>{(*pos)[1].get_or(0.0f), (*pos)[2].get_or(0.0f), (*pos)[3].get_or(1.0f)};
}
void RetryGameScene::hit() {
    call(fn_hit, "RetryGame:hit");
}

bool RetryGameScene::is_finished() {
    return call_r<bool>(fn_is_finished, "RetryGame:is_finished").value_or(true);
}
bool RetryGameScene::is_success() {
    return call_r<bool>(fn_is_success, "RetryGame:is_success").value_or(false);
}
