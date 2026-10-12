#include "retry_message.h"

ResultRetryMessage::ResultRetryMessage() {
    if (!load("ResultRetryMessage", "result_retry_message")) return;
    fn_update      = lua_object["update"];
    fn_draw        = lua_object["draw"];
    fn_is_finished = lua_object["is_finished"];
}

void ResultRetryMessage::update(double current_ms) { call(fn_update, "ResultRetryMessage:update", current_ms); }
void ResultRetryMessage::draw()                    { call(fn_draw, "ResultRetryMessage:draw"); }

// no script: finish at once
bool ResultRetryMessage::is_finished() { return call_r<bool>(fn_is_finished, "ResultRetryMessage:is_finished").value_or(true); }
