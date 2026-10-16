#pragma once

#include <sol/sol.hpp>

#include "../../libs/global_data.h"
#include "../../libs/script.h"

class ResultBackground : public LuaScript {
    sol::protected_function fn_draw;

public:
    ResultBackground() = default;
    ResultBackground(PlayerNum player_num, float width);
    void draw();
};
