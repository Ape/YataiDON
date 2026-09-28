#include "ura_switch.h"
#include <stdexcept>

UraSwitchAnimation::UraSwitchAnimation() {
    texture_change = dynamic_cast<TextureChangeAnimation*>(tex.get_animation(7));
    fade_out = dynamic_cast<FadeAnimation*>(tex.get_animation(8));
    if (!texture_change || !fade_out)
        throw std::runtime_error("UraSwitchAnimation: animation 7/8 has unexpected type");
    t_ura_switch = tex.get_texture("diff_select/ura_switch");
}

void UraSwitchAnimation::start(bool is_backwards) {
    texture_change = dynamic_cast<TextureChangeAnimation*>(tex.get_animation(is_backwards ? 6 : 7));
    if (!texture_change) throw std::runtime_error("UraSwitchAnimation: animation 6/7 is not a TextureChangeAnimation");
    texture_change->start();
    fade_out->start();
}

void UraSwitchAnimation::update(double current_ms) {
    texture_change->update(current_ms);
    fade_out->update(current_ms);
}

void UraSwitchAnimation::draw() {
    tex.draw_texture(t_ura_switch, {.frame=(int)texture_change->attribute, .fade=fade_out->attribute});
}
