#include "dan_game_lua.h"

#include "../../scenes/game_dan.h"

DanGameLua::DanGameLua(const std::string& title) {
    if (!script_manager.lua) {
        return;
    }
    if (!load("DanGameDraw", "dan_game_draw", title)) {
        return;
    }
    fn_draw = lua_object["draw"];
}

void DanGameLua::draw(const DanInfoCache& cache, const std::vector<bool>& exam_failed, int song_index, int dan_color,
                      int dan_rank, double frame_ms) {
    if (!fn_draw.valid()) {
        return;
    }
    sol::state& lua = *script_manager.lua;
    sol::table t = lua.create_table();
    t["song_index"] = song_index;
    t["dan_color"] = dan_color;
    t["dan_rank"] = dan_rank;
    t["frame_ms"] = frame_ms;
    t["remaining_notes"] = cache.remaining_notes;

    sol::table rows = lua.create_table();
    for (size_t i = 0; i < cache.exam_data.size(); i++) {
        const DanExamInfo& d = cache.exam_data[i];
        sol::table r = lua.create_table();
        r["exam_type"] = d.exam_type;
        r["exam_range"] = d.exam_range;
        r["red_value"] = d.red_value;
        r["counter_value"] = d.counter_value;
        r["progress"] = d.progress;
        r["bar_width"] = d.bar_width;
        r["bar_texture"] = d.bar_texture;
        r["bar_state"] = d.bar_state;
        r["gothrough"] = d.gothrough;
        r["song_count"] = d.song_count;
        r["failed"] = i < exam_failed.size() && exam_failed[i];
        sol::table ss = lua.create_table(), sv = lua.create_table(), sp = lua.create_table();
        for (size_t j = 0; j < 3; j++) {
            ss[j + 1] = d.song_state[j];
            sv[j + 1] = d.song_value[j];
            sp[j + 1] = d.song_progress[j];
        }
        r["song_state"] = ss;
        r["song_value"] = sv;
        r["song_progress"] = sp;
        rows[i + 1] = r;
    }
    t["rows"] = rows;
    call(fn_draw, "DanGameLua:draw", t);
}
