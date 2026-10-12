#pragma once

#include "../../libs/script.h"

// Skin-side draw for the texture-only parts of the dan_select screen (DanSelect in dan_select.lua).
class DanSelectScene : public LuaScript {
    sol::protected_function fn_draw_back;
    sol::protected_function fn_draw_confirm;
    sol::protected_function fn_draw_title;
public:
    DanSelectScene();
    bool valid() const { return lua_object.valid() && fn_draw_back.valid(); }

    void draw_back();
    void draw_confirm(float fade, int confirm_index);
    void draw_title();
};
