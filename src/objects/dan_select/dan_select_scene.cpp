#include "dan_select_scene.h"

DanSelectScene::DanSelectScene() {
    if (!load("DanSelect", "dan_select")) {
        return;
    }
    fn_draw_back = lua_object["draw_back"];
    fn_draw_confirm = lua_object["draw_confirm"];
    fn_draw_title = lua_object["draw_title"];
    fn_ribbon = lua_object["ribbon"];
}

void DanSelectScene::draw_back() {
    call(fn_draw_back, "DanSelect:draw_back");
}
void DanSelectScene::draw_confirm(float fade, int confirm_index) {
    call(fn_draw_confirm, "DanSelect:draw_confirm", fade, confirm_index);
}
void DanSelectScene::draw_title() {
    call(fn_draw_title, "DanSelect:draw_title");
}

std::optional<std::array<float, 4>> DanSelectScene::ribbon() {
    auto t = call_r<sol::table>(fn_ribbon, "DanSelect:ribbon");
    if (!t) {
        return std::nullopt;
    }
    return std::array<float, 4>{(*t)[1].get_or(0.0f), (*t)[2].get_or(0.0f), (*t)[3].get_or(0.0f), (*t)[4].get_or(0.0f)};
}
