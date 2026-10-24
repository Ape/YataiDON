#include "fps_counter.h"

#include <algorithm>
#include <functional>

#include "../../libs/gfx/text.h"
#include "../../libs/gfx/texture.h"

FPSCounter::FPSCounter() {
    lastTime = get_current_ms();
    std::ranges::fill(frameTimes, 16.67f);
}

void FPSCounter::update() {
    double currentTime = get_current_ms();
    double deltaTime = currentTime - lastTime;
    lastTime = currentTime;

    frameTimes[currentFrame] = (float)std::clamp(deltaTime, 0.0, 1000.0);
    currentFrame = (currentFrame + 1) % SAMPLE_SIZE;
}

float FPSCounter::get_fps() const {
    const float sum = std::ranges::fold_left(frameTimes, 0.0f, std::plus{});
    float avgFrameTime = sum / SAMPLE_SIZE;
    if (!(avgFrameTime > 0.0f)) {
        return 0.0f;
    }
    return 1000.0f / avgFrameTime;
}

void FPSCounter::draw() {
    int curr_fps = get_fps();
    int size = (int)(20.0f * global_tex.screen_scale);
    float pos = (float)size;

    Color color;
    std::string fps_text = std::to_string(curr_fps) + " FPS";
    Font font = font_manager.get_font(fps_text, size);

    if (curr_fps < 30) {
        color = Color::RED;
    } else if (curr_fps < 60) {
        color = Color::YELLOW;
    } else {
        color = Color::LIME;
    }

    draw_text_ex(font, text_format("%d FPS", curr_fps), Vec2{pos, pos}, pos, 1.0f, color);
}
