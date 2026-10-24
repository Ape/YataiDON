#pragma once

<<<<<<< Updated upstream
#include "../../libs/animation.h"
#include "../../libs/gfx.h"
#include "../../libs/texture.h"
=======
#include "../../libs/math/animation.h"
#include "../../libs/gfx/ray.h"
#include "../../libs/gfx/texture.h"
>>>>>>> Stashed changes

class SongSelectScript;

class SearchBox {
private:
    FadeAnimation* diff_fade_in = nullptr;
    TextureResizeAnimation* bg_resize = nullptr;

public:
    std::string current_search;
    SearchBox();
    void update(double current_ms);
    void draw(SongSelectScript* script); // painted by the skin's SongSelect:draw_search_box
};
