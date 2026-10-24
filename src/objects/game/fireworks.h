#pragma once

#include "../../libs/math/animation.h"
#include "../../libs/gfx/texture.h"

class Fireworks {
private:
    TextureChangeAnimation* explosion_anim = nullptr;
    TextureObject* t_explosion = nullptr;

public:
    Fireworks();

    void update(double current_ms);
    void draw();

    bool is_finished() const;
};
