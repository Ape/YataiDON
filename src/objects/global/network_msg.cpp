#include "network_msg.h"

NetworkMsg::NetworkMsg() {
    if (!load("NetworkMsg", "network_msg")) return;
    available = true;
    fn_update = lua_object["update"];
    fn_draw   = lua_object["draw"];
    fn_show   = lua_object["show"];
    fn_hide   = lua_object["hide"];
}

void NetworkMsg::show(const std::string& label) { call(fn_show, "NetworkMsg:show", label); }
void NetworkMsg::hide() { call(fn_hide, "NetworkMsg:hide"); }
void NetworkMsg::update(double current_ms) { call(fn_update, "NetworkMsg:update", current_ms); }
void NetworkMsg::draw() { call(fn_draw, "NetworkMsg:draw"); }
