#pragma once

#include "color_utils.h"
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

#if defined(PLATFORM_ANDROID) || defined(YATAIDON_PLATFORM_IOS)
#include "sdl_compat.h"
#endif
#include <algorithm>
#include <string>

inline Color fade(Color c, float alpha) {
    return Color(c.r, c.g, c.b, static_cast<uint8_t>(std::clamp(alpha, 0.0f, 1.0f) * 255.0f + 0.5f));
}

inline void draw_rectangle(int x, int y, int w, int h, Color color) {
    ray::DrawRectangle(x, y, w, h, {color.r, color.g, color.b, color.a});
}

inline void draw_rectangle(ray::Rectangle rec, Color color) {
    ray::DrawRectangleRec(rec, {color.r, color.g, color.b, color.a});
}

inline void draw_rectangle_rounded(ray::Rectangle rec, float roundness, int segments, Color color) {
    ray::DrawRectangleRounded(rec, roundness, segments, {color.r, color.g, color.b, color.a});
}

inline void draw_rectangle_rounded_lines_ex(ray::Rectangle rec, float roundness, int segments, float thickness,
                                            Color color) {
    ray::DrawRectangleRoundedLinesEx(rec, roundness, segments, thickness, {color.r, color.g, color.b, color.a});
}

inline void draw_rectangle_lines(int x, int y, int w, int h, Color color) {
    ray::DrawRectangleLines(x, y, w, h, {color.r, color.g, color.b, color.a});
}

inline void draw_rectangle_lines_ex(ray::Rectangle rec, float thickness, Color color) {
    ray::DrawRectangleLinesEx(rec, thickness, {color.r, color.g, color.b, color.a});
}

inline void draw_line(int x0, int y0, int x1, int y1, Color color) {
    ray::DrawLine(x0, y0, x1, y1, {color.r, color.g, color.b, color.a});
}

inline void draw_model(ray::Model model, ray::Vector3 pos, float scale, Color tint) {
    ray::DrawModel(model, pos, scale, {tint.r, tint.g, tint.b, tint.a});
}

inline void draw_text_ex(ray::Font font, const char* text, ray::Vector2 pos, float font_size, float spacing,
                         Color color) {
    ray::DrawTextEx(font, text, pos, font_size, spacing, {color.r, color.g, color.b, color.a});
}

inline void draw_texture_pro(ray::Texture2D tex, ray::Rectangle source, ray::Rectangle dest, ray::Vector2 origin,
                             float rotation, Color tint) {
    ray::DrawTexturePro(tex, source, dest, origin, rotation, {tint.r, tint.g, tint.b, tint.a});
}

inline void draw_text(const char* text, int x, int y, int font_size, Color color) {
    ray::DrawText(text, x, y, font_size, {color.r, color.g, color.b, color.a});
}

inline int measure_text(const char* text, int font_size) {
    return ray::MeasureText(text, font_size);
}

// --- Window ---------------------------------------------------------------------------------
namespace window_flag {
    inline constexpr unsigned RESIZABLE = ray::FLAG_WINDOW_RESIZABLE;
    inline constexpr unsigned VSYNC = ray::FLAG_VSYNC_HINT;
    inline constexpr unsigned ALWAYS_RUN = ray::FLAG_WINDOW_ALWAYS_RUN;
} // namespace window_flag

inline void set_window_flags(unsigned flags) {
    ray::SetConfigFlags(flags);
}

inline void quiet_raylib_log() {
    ray::SetTraceLogLevel(ray::LOG_ERROR);
}

inline void init_window(int width, int height, const char* title) {
    ray::InitWindow(width, height, title);
}

inline void close_window() {
    ray::CloseWindow();
}

inline bool window_should_close() {
    return ray::WindowShouldClose();
}

inline void disable_exit_key() {
    ray::SetExitKey(ray::KEY_NULL);
}

inline void toggle_fullscreen() {
    ray::ToggleFullscreen();
}

inline void toggle_borderless_windowed() {
    ray::ToggleBorderlessWindowed();
}

inline bool is_window_fullscreen() {
    return ray::IsWindowFullscreen();
}

inline void set_window_size(int width, int height) {
    ray::SetWindowSize(width, height);
}

inline int get_screen_width() {
    return ray::GetScreenWidth();
}

inline int get_screen_height() {
    return ray::GetScreenHeight();
}

inline int get_render_width() {
    return ray::GetRenderWidth();
}

inline int get_render_height() {
    return ray::GetRenderHeight();
}

inline int get_monitor_refresh_rate() {
    return ray::GetMonitorRefreshRate(ray::GetCurrentMonitor());
}

inline void begin_drawing() {
    ray::BeginDrawing();
}

inline void end_drawing() {
    ray::EndDrawing();
}

inline void swap_screen_buffer() {
    ray::SwapScreenBuffer();
}

namespace texture_filter {
    inline constexpr ray::TextureFilter BILINEAR = ray::TEXTURE_FILTER_BILINEAR;
    inline constexpr ray::TextureFilter TRILINEAR = ray::TEXTURE_FILTER_TRILINEAR;
}

inline ray::Texture2D load_texture(const char* path) {
    return ray::LoadTexture(path);
}

inline ray::Texture2D load_texture_from_image(ray::Image image) {
    return ray::LoadTextureFromImage(image);
}

inline void unload_texture(ray::Texture2D tex) {
    ray::UnloadTexture(tex);
}

inline void set_texture_filter(ray::Texture2D tex, ray::TextureFilter filter) {
    ray::SetTextureFilter(tex, filter);
}

inline void gen_texture_mipmaps(ray::Texture2D& tex) {
    ray::GenTextureMipmaps(&tex);
}

inline ray::Texture2D load_texture_mipmapped(const char* path,
                                             ray::TextureFilter filter = texture_filter::TRILINEAR) {
    ray::Texture2D tex = ray::LoadTexture(path);
    gen_texture_mipmaps(tex);
    set_texture_filter(tex, filter);
    return tex;
}

inline ray::Vector2 measure_text_ex(const ray::Font& font, const char* text, float font_size, float spacing) {
    return ray::MeasureTextEx(font, text, font_size, spacing);
}

template <typename... Args>
inline const char* text_format(const char* fmt, Args... args) {
    return ray::TextFormat(fmt, args...);
}

inline ray::Font load_font_ex(const char* path, int font_size, int* codepoints, int codepoint_count) {
    return ray::LoadFontEx(path, font_size, codepoints, codepoint_count);
}

inline void unload_font(ray::Font font) {
    ray::UnloadFont(font);
}

inline ray::Font get_font_default() {
    return ray::GetFontDefault();
}

inline int get_glyph_index(const ray::Font& font, int codepoint) {
    return ray::GetGlyphIndex(font, codepoint);
}

inline int get_codepoint_next(const char* text, int* codepoint_size) {
    return ray::GetCodepointNext(text, codepoint_size);
}

inline const char* codepoint_to_utf8(int codepoint, int* utf8_size) {
    return ray::CodepointToUTF8(codepoint, utf8_size);
}

namespace blend_mode {
    inline constexpr int ADDITIVE = ray::BLEND_ADDITIVE;
    inline constexpr int CUSTOM_SEPARATE = ray::BLEND_CUSTOM_SEPARATE;
} // namespace blend_mode

inline void begin_scissor_mode(int x, int y, int width, int height) {
    ray::BeginScissorMode(x, y, width, height);
}

inline void end_scissor_mode() {
    ray::EndScissorMode();
}

inline void begin_blend_mode(int mode) {
    ray::BeginBlendMode(mode);
}

inline void end_blend_mode() {
    ray::EndBlendMode();
}

inline void begin_mode_2d(ray::Camera2D camera) {
    ray::BeginMode2D(camera);
}

inline void end_mode_2d() {
    ray::EndMode2D();
}

inline void begin_mode_3d(ray::Camera3D camera) {
    ray::BeginMode3D(camera);
}

inline void end_mode_3d() {
    ray::EndMode3D();
}

inline ray::Vector2 get_screen_to_world_2d(ray::Vector2 position, ray::Camera2D camera) {
    return ray::GetScreenToWorld2D(position, camera);
}

namespace shader_uniform {
    inline constexpr int FLOAT = ray::SHADER_UNIFORM_FLOAT;
    inline constexpr int VEC2 = ray::SHADER_UNIFORM_VEC2;
    inline constexpr int VEC3 = ray::SHADER_UNIFORM_VEC3;
} // namespace shader_uniform

inline void unload_shader(ray::Shader shader) {
    ray::UnloadShader(shader);
}

inline bool is_shader_valid(ray::Shader shader) {
    return ray::IsShaderValid(shader);
}

inline int get_shader_location(ray::Shader shader, const char* name) {
    return ray::GetShaderLocation(shader, name);
}

inline void set_shader_value(ray::Shader shader, int loc, const void* value, int uniform_type) {
    ray::SetShaderValue(shader, loc, value, uniform_type);
}

inline void begin_shader_mode(ray::Shader shader) {
    ray::BeginShaderMode(shader);
}

inline void end_shader_mode() {
    ray::EndShaderMode();
}

namespace pixel_format {
    inline constexpr int RGB8 = ray::PIXELFORMAT_UNCOMPRESSED_R8G8B8;
    inline constexpr int RGBA8 = ray::PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
} // namespace pixel_format

inline ray::Image load_image(const char* path) {
    return ray::LoadImage(path);
}

inline ray::Image load_image_from_texture(ray::Texture2D tex) {
    return ray::LoadImageFromTexture(tex);
}

inline ray::Image load_image_from_screen() {
    return ray::LoadImageFromScreen();
}

inline void unload_image(ray::Image image) {
    ray::UnloadImage(image);
}

inline bool export_image(ray::Image image, const char* path) {
    return ray::ExportImage(image, path);
}

inline ray::Image image_copy(ray::Image image) {
    return ray::ImageCopy(image);
}

inline ray::Image image_from_image(ray::Image image, ray::Rectangle rec) {
    return ray::ImageFromImage(image, rec);
}

inline void image_format(ray::Image* image, int new_format) {
    ray::ImageFormat(image, new_format);
}

inline void image_crop(ray::Image* image, ray::Rectangle crop) {
    ray::ImageCrop(image, crop);
}

inline void image_rotate_cw(ray::Image* image) {
    ray::ImageRotateCW(image);
}

inline void image_draw_image_pro(ray::Image* dst, ray::Image src, ray::Rectangle src_rec, ray::Rectangle dst_rec,
                                 ray::Vector2 origin, float rotation, Color tint) {
    ray::ImageDrawImagePro(dst, src, src_rec, dst_rec, origin, rotation, {tint.r, tint.g, tint.b, tint.a});
}

inline ray::Rectangle get_image_alpha_border(ray::Image image, float threshold) {
    return ray::GetImageAlphaBorder(image, threshold);
}

inline int get_pixel_data_size(int width, int height, int format) {
    return ray::GetPixelDataSize(width, height, format);
}

inline void update_texture(ray::Texture2D tex, const void* pixels) {
    ray::UpdateTexture(tex, pixels);
}

inline bool is_texture_valid(ray::Texture2D tex) {
    return ray::IsTextureValid(tex);
}

inline unsigned char* load_file_data(const char* path, int* size) {
    return ray::LoadFileData(path, size);
}

inline void unload_file_data(unsigned char* data) {
    ray::UnloadFileData(data);
}

inline void clear_background(Color color) {
    ray::ClearBackground({color.r, color.g, color.b, color.a});
}

inline ray::Image gen_image_color(int w, int h, Color color) {
    return ray::GenImageColor(w, h, {color.r, color.g, color.b, color.a});
}

inline ray::Shader load_shader(const char* vs_path, const char* fs_path) {
#if defined(PLATFORM_ANDROID) || defined(YATAIDON_PLATFORM_IOS)
    auto repath = [](const char* p) -> std::string {
        if (!p) {
            return "";
        }
        std::string s(p);
        const std::string prefix = "shader/";
        const std::string es_prefix = "shader/es/";
        if (s.compare(0, es_prefix.size(), es_prefix) == 0) {
            return s;
        }
        if (s.size() >= prefix.size() && s.substr(0, prefix.size()) == prefix) {
            s.insert(prefix.size(), "es/");
        }
        return s;
    };

    auto read_asset = [](const std::string& path) -> std::string {
        if (path.empty()) {
            return "";
        }
        SDL_IOStream* io = SDL_IOFromFile(path.c_str(), "r");
        if (!io) {
            return "";
        }
        Sint64 size = SDL_GetIOSize(io);
        if (size <= 0) {
            SDL_CloseIO(io);
            return "";
        }
        std::string buf(static_cast<size_t>(size), '\0');
        size_t got = SDL_ReadIO(io, buf.data(), static_cast<size_t>(size));
        SDL_CloseIO(io);
        if (got != static_cast<size_t>(size)) {
            return "";
        }
        return buf;
    };

    std::string vs_repath = vs_path ? repath(vs_path) : std::string{};
    std::string fs_repath = fs_path ? repath(fs_path) : std::string{};
    std::string vs_src = vs_path ? read_asset(vs_repath) : std::string{};
    std::string fs_src = fs_path ? read_asset(fs_repath) : std::string{};
    return ray::LoadShaderFromMemory(vs_src.empty() ? nullptr : vs_src.c_str(),
                                     fs_src.empty() ? nullptr : fs_src.c_str());
#endif
#ifdef __EMSCRIPTEN__
    auto repath = [](const char* p) -> std::string {
        if (!p) {
            return "";
        }
        std::string s(p);
        const std::string prefix = "shader/";
        const std::string es_prefix = "shader/es/";
        if (s.compare(0, es_prefix.size(), es_prefix) == 0) {
            return s;
        }
        if (s.size() >= prefix.size() && s.substr(0, prefix.size()) == prefix) {
            s.insert(prefix.size(), "es/");
        }
        return s;
    };
    std::string vs_repath = vs_path ? repath(vs_path) : std::string{};
    std::string fs_repath = fs_path ? repath(fs_path) : std::string{};
    return ray::LoadShader(vs_path ? vs_repath.c_str() : nullptr, fs_path ? fs_repath.c_str() : nullptr);
#endif
    return ray::LoadShader(vs_path, fs_path);
}
