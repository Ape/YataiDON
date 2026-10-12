#include "dan_select_scene.h"

DanSelectScene::DanSelectScene() {
    if (!load("DanSelect", "dan_select")) return;
    fn_draw_back    = lua_object["draw_back"];
    fn_draw_confirm = lua_object["draw_confirm"];
    fn_draw_title   = lua_object["draw_title"];
}

void DanSelectScene::draw_back()                              { call(fn_draw_back, "DanSelect:draw_back"); }
void DanSelectScene::draw_confirm(float fade, int confirm_index) { call(fn_draw_confirm, "DanSelect:draw_confirm", fade, confirm_index); }
void DanSelectScene::draw_title()                             { call(fn_draw_title, "DanSelect:draw_title"); }
