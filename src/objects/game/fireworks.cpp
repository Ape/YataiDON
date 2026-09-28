#include "fireworks.h"
#include <algorithm>
#include <cmath>

static constexpr int GOGO_EXPLOSION_ANIM_ID = 23;

Fireworks::Fireworks() {
    explosion_anim = dynamic_cast<TextureChangeAnimation*>(tex.get_animation(GOGO_EXPLOSION_ANIM_ID, true));
    if (!explosion_anim) {
        throw std::runtime_error("Animation " + std::to_string(GOGO_EXPLOSION_ANIM_ID) + " is not a TextureChangeAnimation");
    }

    explosion_anim->start();
    t_explosion = tex.get_texture("gogo_time/explosion");
}

void Fireworks::update(double current_ms) {
    if (!explosion_anim) return;
    explosion_anim->update(current_ms);
}

void Fireworks::draw() {
    if (!explosion_anim) return;
    if (!explosion_anim->is_finished) {
        int slots = 5;
        int mirror_from = -1;
        constexpr int MAX_EXPLOSION_SLOTS = 32;
        if (const SkinInfo* s = tex.skin_entry("gogo_explosion_slots"); s && std::isfinite(s->x) && s->x > 0) {
            slots = static_cast<int>(std::clamp(s->x, 1.0f, static_cast<float>(MAX_EXPLOSION_SLOTS)));
            if (std::isfinite(s->y) && s->y > 0) mirror_from = static_cast<int>(std::min(s->y, static_cast<float>(slots)));
        }
        for (int i = 0; i < slots; i++) {
            tex.draw_texture(t_explosion, {
                .frame = (int)explosion_anim->attribute,
                .mirror = (mirror_from >= 0 && i >= mirror_from) ? Mirror::HORIZONTAL : Mirror::NONE,
                .index = i});
        }
    }
}

bool Fireworks::is_finished() {
    return !explosion_anim || explosion_anim->is_finished;
}
