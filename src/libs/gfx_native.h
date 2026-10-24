#pragma once

// Transitional: raylib types not yet wrapped by gfx.h (Image, Font, Model, ...).
// Include only where those are still used directly.

#include "gfx.h"

namespace ray {
    extern "C" {
#include <raylib.h>
// raylib color macros clash with Color::WHITE etc.
#undef WHITE
#undef BLACK
#undef BLANK
#undef BLUE
#undef GRAY
#undef GREEN
#undef LIGHTGRAY
#undef LIME
#undef ORANGE
#undef RED
#undef SKYBLUE
#undef YELLOW
    }
} // namespace ray

namespace ray {
    inline bool operator==(const Color& a, const Color& b) {
        return a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a;
    }

    inline bool operator!=(const Color& a, const Color& b) {
        return !(a == b);
    }
} // namespace ray
