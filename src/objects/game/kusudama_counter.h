#pragma once

#include "../../libs/animation.h"

#include "../../libs/texture.h"

class KusudamaCounter {
private:
    int balloon_total;
    int balloon_count;
    bool is_popped;
    TextureObject* t_kusudama = nullptr;
    TextureObject* t_renda = nullptr;
    TextureObject* t_counter = nullptr;
    MoveAnimation* move = nullptr;
    MoveAnimation* renda_move = nullptr;
    FadeAnimation* renda_fade_in = nullptr;
    FadeAnimation* renda_fade_out = nullptr;
    TextStretchAnimation* stretch = nullptr;
    TextureResizeAnimation* breathing = nullptr;
    MoveAnimation* renda_breathe = nullptr;
    TextureChangeAnimation* open = nullptr;
    FadeAnimation* fade_out = nullptr;
public:
    KusudamaCounter(int total);

    void update_count(int count);

    void update(double current_ms, int count);

    void draw();

    bool is_finished() const;

    bool has_popped() const;
};
