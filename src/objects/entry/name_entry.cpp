#include "name_entry.h"

NameEntry::NameEntry(bool is_2p) {
    if (!load("NameEntry", "name_entry", is_2p)) {
        return;
    }
    fn_update = lua_object["update"];
    fn_input = lua_object["input"];
    fn_draw = lua_object["draw"];
    fn_is_done = lua_object["is_done"];
    fn_get_name = lua_object["get_name"];
}

void NameEntry::update(double current_ms) {
    call(fn_update, "NameEntry:update", current_ms);
}
void NameEntry::input(Action action) {
    call(fn_input, "NameEntry:input", static_cast<int>(action));
}
void NameEntry::draw() {
    call(fn_draw, "NameEntry:draw");
}
bool NameEntry::is_done() {
    return call_r<bool>(fn_is_done, "NameEntry:is_done").value_or(true);
}
std::string NameEntry::get_name() {
    return call_r<std::string>(fn_get_name, "NameEntry:get_name").value_or("");
}
