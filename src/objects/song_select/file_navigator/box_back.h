#pragma once

#include "box_base.h"
#include "../../../libs/color_utils.h"

class BackBox : public BaseBox {
public:
    static constexpr Color COLOR = Color(170, 115, 35);

    BackBox(const fs::path& path, const BoxDef& box_def);

    const char* lua_kind() const override { return "back"; }
};
