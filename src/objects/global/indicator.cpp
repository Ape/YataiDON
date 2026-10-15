#include "indicator.h"

Indicator::Indicator(State state) {
    if (!load("Indicator", "indicator", static_cast<int>(state))) return;
    fn_update = lua_object["update"];
    fn_draw   = lua_object["draw"];
    fn_draw_top = lua_object["draw_top"];
    fn_screen_pos = lua_object["screen_pos"];
}

void Indicator::update(double current_ms)          { call(fn_update, "Indicator:update", current_ms); }
void Indicator::draw(float x, float y, float fade) { call(fn_draw, "Indicator:draw", x, y, fade); }
void Indicator::draw_on(const std::string& screen, float fade) {
    auto pos = call_r<sol::table>(fn_screen_pos, "Indicator:screen_pos", screen);
    if (!pos) return;
    draw((*pos)[1].get_or(0.0f), (*pos)[2].get_or(0.0f), fade);
}
void Indicator::draw_top()                         { call(fn_draw_top, "Indicator:draw_top"); }
