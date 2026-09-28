#include "result_crown.h"

ResultCrown::ResultCrown(CrownType crown_type, bool is_2p) {
    if (!load("ResultCrown", "result_crown", crown_type, is_2p)) return;
    fn_update    = lua_object["update"];
    fn_draw      = lua_object["draw"];
    fn_is_settled = lua_object["is_settled"];
}

void ResultCrown::update(double current_ms) {
    call(fn_update, "ResultCrown:update", current_ms);
}

void ResultCrown::draw() {
    call(fn_draw, "ResultCrown:draw");
}

bool ResultCrown::is_settled() {
    if (!fn_is_settled.valid()) return true;
    return call_r<bool>(fn_is_settled, "ResultCrown:is_settled").value_or(true);
}
