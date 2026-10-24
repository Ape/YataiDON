#pragma once

// Backend-neutral graphics interface. No backend header is included here; the
// implementation lives in gfx_raylib.cpp.

#include <cstdint>

#include "gfx/color_utils.h"
#include "gfx/gfx_types.h"
#include "math/math_types.h"
#if defined(PLATFORM_ANDROID) || defined(YATAIDON_PLATFORM_IOS)
#include "gfx/sdl_compat.h"
#endif
#include <algorithm>
#include <string>

inline constexpr float deg2rad = 3.14159265358979323846f / 180.0f;

inline Color fade(Color c, float alpha) {
    return Color(c.r, c.g, c.b, static_cast<uint8_t>(std::clamp(alpha, 0.0f, 1.0f) * 255.0f + 0.5f));
}

namespace window_flag {
    inline constexpr unsigned RESIZABLE = 0x00000004;
    inline constexpr unsigned VSYNC = 0x00000040;
    inline constexpr unsigned ALWAYS_RUN = 0x00000100;
} // namespace window_flag

namespace texture_filter {
    inline constexpr int BILINEAR = 1;
    inline constexpr int TRILINEAR = 2;
} // namespace texture_filter

namespace texture_wrap {
    inline constexpr int CLAMP = 1;
} // namespace texture_wrap

namespace blend_mode {
    inline constexpr int ADDITIVE = 1;
    inline constexpr int CUSTOM_SEPARATE = 7;
} // namespace blend_mode

namespace shader_uniform {
    inline constexpr int FLOAT = 0;
    inline constexpr int VEC2 = 1;
    inline constexpr int VEC3 = 2;
} // namespace shader_uniform

namespace pixel_format {
    inline constexpr int RGB8 = 4;
    inline constexpr int RGBA8 = 7;
} // namespace pixel_format

namespace cull_face {
    inline constexpr int FRONT = 0;
    inline constexpr int BACK = 1;
} // namespace cull_face

namespace material_map {
    inline constexpr int DIFFUSE = 0;
}

// printf-style; returns a rotating static buffer like raylib's TextFormat.
__attribute__((format(printf, 1, 2))) const char* text_format(const char* fmt, ...);

void draw_rectangle(int x, int y, int w, int h, Color color);
void draw_rectangle(Rect rec, Color color);
void draw_rectangle_rounded(Rect rec, float roundness, int segments, Color color);
void draw_rectangle_rounded_lines_ex(Rect rec, float roundness, int segments, float thickness, Color color);
void draw_rectangle_lines(int x, int y, int w, int h, Color color);
void draw_rectangle_lines_ex(Rect rec, float thickness, Color color);
void draw_line(int x0, int y0, int x1, int y1, Color color);
void draw_model(Model model, Vec3 pos, float scale, Color tint);
void draw_text_ex(Font font, const char* text, Vec2 pos, float font_size, float spacing, Color color);
void draw_texture_pro(Texture tex, Rect source, Rect dest, Vec2 origin, float rotation, Color tint);
void draw_text(const char* text, int x, int y, int font_size, Color color);
int measure_text(const char* text, int font_size);
void set_window_flags(unsigned flags);
void quiet_raylib_log();
void init_window(int width, int height, const char* title);
void close_window();
bool window_should_close();
void poll_window_events();
void disable_exit_key();
void toggle_fullscreen();
void toggle_borderless_windowed();
bool is_window_fullscreen();
void set_window_size(int width, int height);
int get_screen_width();
int get_screen_height();
int get_render_width();
int get_render_height();
int get_monitor_refresh_rate();
void begin_drawing();
void end_drawing();
void swap_screen_buffer();
Texture load_texture(const char* path);
Texture load_texture_from_image(Image image);
void unload_texture(Texture tex);
void set_texture_filter(Texture tex, int filter);
void set_texture_wrap(Texture tex, int wrap);
void gen_texture_mipmaps(Texture& tex);
Texture load_texture_mipmapped(const char* path, int filter = texture_filter::TRILINEAR);
Vec2 measure_text_ex(const Font& font, const char* text, float font_size, float spacing);
Font load_font_ex(const char* path, int font_size, int* codepoints, int codepoint_count);
void unload_font(Font font);
// Rasterizes codepoints; the returned array (and each glyph's image data) is
// malloc'd: free images with unload_image and the array with std::free.
GlyphInfo* load_font_data(const unsigned char* data, int data_size, int font_size, const int* codepoints,
                          int codepoint_count, int* glyph_count);
// Packs glyph images into an atlas; *recs receives a malloc'd array of glyph_count rects.
Image gen_image_font_atlas(const GlyphInfo* glyphs, Rect** recs, int glyph_count, int font_size, int padding);
Font get_font_default();
int get_glyph_index(const Font& font, int codepoint);
int get_codepoint_next(const char* text, int* codepoint_size);
const char* codepoint_to_utf8(int codepoint, int* utf8_size);
void begin_scissor_mode(int x, int y, int width, int height);
void end_scissor_mode();
void begin_blend_mode(int mode);
// Factors for blend_mode::CUSTOM_SEPARATE: color src_alpha/one_minus_src_alpha,
// alpha one/one_minus_src_alpha (keeps render-target alpha correct).
void set_separate_alpha_blend();
void set_backface_culling(bool enabled);
void set_cull_face(int face);
void end_blend_mode();
void begin_mode_2d(Camera2D camera);
void end_mode_2d();
void begin_mode_3d(Camera3D camera);
void end_mode_3d();
Vec2 get_screen_to_world_2d(Vec2 position, Camera2D camera);
Vec2 get_world_to_screen_2d(Vec2 position, Camera2D camera);
void unload_shader(Shader shader);
bool is_shader_valid(Shader shader);
int get_shader_location(Shader shader, const char* name);
void set_shader_value(Shader shader, int loc, const void* value, int uniform_type);
void begin_shader_mode(Shader shader);
void end_shader_mode();
Image load_image(const char* path);
Image load_image_from_texture(Texture tex);
Image load_image_from_screen();
void unload_image(Image image);
bool export_image(Image image, const char* path);
Image image_copy(Image image);
Image image_from_image(Image image, Rect rec);
void image_format(Image* image, int new_format);
void image_crop(Image* image, Rect crop);
void image_rotate_cw(Image* image);
void image_draw_image_pro(Image* dst, Image src, Rect src_rec, Rect dst_rec, Vec2 origin, float rotation, Color tint);
Rect get_image_alpha_border(Image image, float threshold);
int get_pixel_data_size(int width, int height, int format);
void update_texture(Texture tex, const void* pixels);
bool is_texture_valid(Texture tex);
unsigned char* load_file_data(const char* path, int* size);
void unload_file_data(unsigned char* data);
RenderTarget load_render_texture(int width, int height);
void unload_render_texture(RenderTarget target);
void begin_texture_mode(RenderTarget target);
void end_texture_mode();
Model load_model(const char* path);
void unload_model(Model model);
ModelAnimation* load_model_animations(const char* path, int* count);
void unload_model_animations(ModelAnimation* anims, int count);
void update_model_animation(Model model, ModelAnimation anim, float frame);
void draw_mesh(Mesh mesh, Material material, Mat4 transform);
void update_mesh_buffer(Mesh mesh, int index, const void* data, int data_size, int offset);
void set_material_texture(Material* material, int map_type, Texture texture);
void clear_background(Color color);
Image gen_image_color(int w, int h, Color color);
Shader load_shader(const char* vs_path, const char* fs_path);
