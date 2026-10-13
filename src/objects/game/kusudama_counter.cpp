#include "kusudama_counter.h"

KusudamaCounter::KusudamaCounter(int total) {
    if (!load("KusudamaCounter", "kusudama_counter", total)) return;
    fn_update   = lua_object["update"];
    fn_miss     = lua_object["miss"];
    fn_draw     = lua_object["draw"];
    fn_popped   = lua_object["has_popped"];
    fn_missed   = lua_object["has_missed"];
    fn_finished = lua_object["is_finished"];
    fx = lua_object["fx"].get_or(false);
}

void KusudamaCounter::update(double current_ms, int count) {
    call(fn_update, "KusudamaCounter:update", current_ms, count);
}

void KusudamaCounter::draw() {
    call(fn_draw, "KusudamaCounter:draw");
}

void KusudamaCounter::miss(double current_ms) {
    call(fn_miss, "KusudamaCounter:miss", current_ms);
}

bool KusudamaCounter::has_popped() {
    return call_r<bool>(fn_popped, "KusudamaCounter:has_popped").value_or(false);
}

bool KusudamaCounter::has_missed() {
    return call_r<bool>(fn_missed, "KusudamaCounter:has_missed").value_or(false);
}

bool KusudamaCounter::is_finished() {
    return call_r<bool>(fn_finished, "KusudamaCounter:is_finished").value_or(true);
}
