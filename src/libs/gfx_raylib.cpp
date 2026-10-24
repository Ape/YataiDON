#include <bit>
#include <cstdarg>
#include <cstddef>
#include <cstdio>

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
#include <rlgl.h>
    }
} // namespace ray

// Project data types share raylib's layout; convert by bit copy / pointer cast.
template<class To, class From>
static To to(const From& v) {
    static_assert(sizeof(To) == sizeof(From));
    return std::bit_cast<To>(v);
}
template<class To, class From>
static To* to_ptr(From* p) {
    static_assert(sizeof(To) == sizeof(From));
    return reinterpret_cast<To*>(p);
}
template<class To, class From>
static const To* to_ptr(const From* p) {
    static_assert(sizeof(To) == sizeof(From));
    return reinterpret_cast<const To*>(p);
}

#define SAME_OFFSET(T, f) static_assert(offsetof(T, f) == offsetof(ray::T, f))
SAME_OFFSET(Image, format);
SAME_OFFSET(GlyphInfo, image);
SAME_OFFSET(Font, glyphs);
SAME_OFFSET(Transform, scale);
SAME_OFFSET(BoneInfo, parent);
SAME_OFFSET(Mesh, boneCount);
SAME_OFFSET(Mesh, vboId);
SAME_OFFSET(MaterialMap, value);
SAME_OFFSET(Material, params);
SAME_OFFSET(ModelSkeleton, bindPose);
SAME_OFFSET(Model, skeleton);
SAME_OFFSET(Model, boneMatrices);
SAME_OFFSET(ModelAnimation, keyframePoses);
static_assert(sizeof(Mesh) == sizeof(ray::Mesh) && sizeof(Model) == sizeof(ray::Model) &&
              sizeof(Material) == sizeof(ray::Material) && sizeof(ModelAnimation) == sizeof(ray::ModelAnimation));
#undef SAME_OFFSET

static_assert(window_flag::RESIZABLE == ray::FLAG_WINDOW_RESIZABLE);
static_assert(window_flag::VSYNC == ray::FLAG_VSYNC_HINT);
static_assert(window_flag::ALWAYS_RUN == ray::FLAG_WINDOW_ALWAYS_RUN);
static_assert(texture_filter::BILINEAR == ray::TEXTURE_FILTER_BILINEAR);
static_assert(texture_filter::TRILINEAR == ray::TEXTURE_FILTER_TRILINEAR);
static_assert(texture_wrap::CLAMP == ray::TEXTURE_WRAP_CLAMP);
static_assert(blend_mode::ADDITIVE == ray::BLEND_ADDITIVE);
static_assert(blend_mode::CUSTOM_SEPARATE == ray::BLEND_CUSTOM_SEPARATE);
static_assert(shader_uniform::FLOAT == ray::SHADER_UNIFORM_FLOAT);
static_assert(shader_uniform::VEC2 == ray::SHADER_UNIFORM_VEC2);
static_assert(shader_uniform::VEC3 == ray::SHADER_UNIFORM_VEC3);
static_assert(pixel_format::RGB8 == ray::PIXELFORMAT_UNCOMPRESSED_R8G8B8);
static_assert(pixel_format::RGBA8 == ray::PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
static_assert(material_map::DIFFUSE == ray::MATERIAL_MAP_DIFFUSE);
static_assert(cull_face::FRONT == ray::RL_CULL_FACE_FRONT);
static_assert(cull_face::BACK == ray::RL_CULL_FACE_BACK);

const char* text_format(const char* fmt, ...) {
    static char buffers[4][1024];
    static int index = 0;
    char* out = buffers[index];
    index = (index + 1) % 4;
    va_list args;
    va_start(args, fmt);
    vsnprintf(out, sizeof(buffers[0]), fmt, args);
    va_end(args);
    return out;
}

void draw_rectangle(int x, int y, int w, int h, Color color) {
    ray::DrawRectangle(x, y, w, h, {color.r, color.g, color.b, color.a});
}

void draw_rectangle(Rect rec, Color color) {
    ray::DrawRectangleRec(rec, {color.r, color.g, color.b, color.a});
}

void draw_rectangle_rounded(Rect rec, float roundness, int segments, Color color) {
    ray::DrawRectangleRounded(rec, roundness, segments, {color.r, color.g, color.b, color.a});
}

void draw_rectangle_rounded_lines_ex(Rect rec, float roundness, int segments, float thickness, Color color) {
    ray::DrawRectangleRoundedLinesEx(rec, roundness, segments, thickness, {color.r, color.g, color.b, color.a});
}

void draw_rectangle_lines(int x, int y, int w, int h, Color color) {
    ray::DrawRectangleLines(x, y, w, h, {color.r, color.g, color.b, color.a});
}

void draw_rectangle_lines_ex(Rect rec, float thickness, Color color) {
    ray::DrawRectangleLinesEx(rec, thickness, {color.r, color.g, color.b, color.a});
}

void draw_line(int x0, int y0, int x1, int y1, Color color) {
    ray::DrawLine(x0, y0, x1, y1, {color.r, color.g, color.b, color.a});
}

void draw_model(Model model, Vec3 pos, float scale, Color tint) {
    ray::DrawModel(to<ray::Model>(model), pos, scale, {tint.r, tint.g, tint.b, tint.a});
}

void draw_text_ex(Font font, const char* text, Vec2 pos, float font_size, float spacing, Color color) {
    ray::DrawTextEx(to<ray::Font>(font), text, pos, font_size, spacing, {color.r, color.g, color.b, color.a});
}

void draw_texture_pro(Texture tex, Rect source, Rect dest, Vec2 origin, float rotation, Color tint) {
    ray::DrawTexturePro(tex, source, dest, origin, rotation, {tint.r, tint.g, tint.b, tint.a});
}

void draw_text(const char* text, int x, int y, int font_size, Color color) {
    ray::DrawText(text, x, y, font_size, {color.r, color.g, color.b, color.a});
}

int measure_text(const char* text, int font_size) {
    return ray::MeasureText(text, font_size);
}

void set_window_flags(unsigned flags) {
    ray::SetConfigFlags(flags);
}

void quiet_raylib_log() {
    ray::SetTraceLogLevel(ray::LOG_ERROR);
}

void init_window(int width, int height, const char* title) {
    ray::InitWindow(width, height, title);
}

void close_window() {
    ray::CloseWindow();
}

void poll_window_events() {
    ray::PollInputEvents();
}

bool window_should_close() {
    return ray::WindowShouldClose();
}

void disable_exit_key() {
    ray::SetExitKey(ray::KEY_NULL);
}

void toggle_fullscreen() {
    ray::ToggleFullscreen();
}

void toggle_borderless_windowed() {
    ray::ToggleBorderlessWindowed();
}

bool is_window_fullscreen() {
    return ray::IsWindowFullscreen();
}

void set_window_size(int width, int height) {
    ray::SetWindowSize(width, height);
}

int get_screen_width() {
    return ray::GetScreenWidth();
}

int get_screen_height() {
    return ray::GetScreenHeight();
}

int get_render_width() {
    return ray::GetRenderWidth();
}

int get_render_height() {
    return ray::GetRenderHeight();
}

int get_monitor_refresh_rate() {
    return ray::GetMonitorRefreshRate(ray::GetCurrentMonitor());
}

void begin_drawing() {
    ray::BeginDrawing();
}

void end_drawing() {
    ray::EndDrawing();
}

void swap_screen_buffer() {
    ray::SwapScreenBuffer();
}

Texture load_texture(const char* path) {
    return ray::LoadTexture(path);
}

Texture load_texture_from_image(Image image) {
    return ray::LoadTextureFromImage(to<ray::Image>(image));
}

void unload_texture(Texture tex) {
    ray::UnloadTexture(tex);
}

void set_texture_filter(Texture tex, int filter) {
    ray::SetTextureFilter(tex, filter);
}

void set_texture_wrap(Texture tex, int wrap) {
    ray::SetTextureWrap(tex, wrap);
}

void gen_texture_mipmaps(Texture& tex) {
    ray::Texture2D native = tex;
    ray::GenTextureMipmaps(&native);
    tex = native;
}

Texture load_texture_mipmapped(const char* path, int filter) {
    Texture tex = ray::LoadTexture(path);
    gen_texture_mipmaps(tex);
    set_texture_filter(tex, filter);
    return tex;
}

Vec2 measure_text_ex(const Font& font, const char* text, float font_size, float spacing) {
    return ray::MeasureTextEx(to<ray::Font>(font), text, font_size, spacing);
}

Font load_font_ex(const char* path, int font_size, int* codepoints, int codepoint_count) {
    return to<Font>(ray::LoadFontEx(path, font_size, codepoints, codepoint_count));
}

void unload_font(Font font) {
    ray::UnloadFont(to<ray::Font>(font));
}

Font get_font_default() {
    return to<Font>(ray::GetFontDefault());
}

GlyphInfo* load_font_data(const unsigned char* data, int data_size, int font_size, const int* codepoints,
                          int codepoint_count, int* glyph_count) {
    return to_ptr<GlyphInfo>(
        ray::LoadFontData(data, data_size, font_size, codepoints, codepoint_count, ray::FONT_DEFAULT, glyph_count));
}

Image gen_image_font_atlas(const GlyphInfo* glyphs, Rect** recs, int glyph_count, int font_size, int padding) {
    return to<Image>(ray::GenImageFontAtlas(to_ptr<ray::GlyphInfo>(glyphs), reinterpret_cast<ray::Rectangle**>(recs),
                                            glyph_count, font_size, padding, 0));
}

int get_glyph_index(const Font& font, int codepoint) {
    return ray::GetGlyphIndex(to<ray::Font>(font), codepoint);
}

int get_codepoint_next(const char* text, int* codepoint_size) {
    return ray::GetCodepointNext(text, codepoint_size);
}

const char* codepoint_to_utf8(int codepoint, int* utf8_size) {
    return ray::CodepointToUTF8(codepoint, utf8_size);
}

void begin_scissor_mode(int x, int y, int width, int height) {
    ray::BeginScissorMode(x, y, width, height);
}

void end_scissor_mode() {
    ray::EndScissorMode();
}

void begin_blend_mode(int mode) {
    ray::BeginBlendMode(mode);
}

void set_separate_alpha_blend() {
    ray::rlSetBlendFactorsSeparate(RL_SRC_ALPHA, RL_ONE_MINUS_SRC_ALPHA, RL_ONE, RL_ONE_MINUS_SRC_ALPHA, RL_FUNC_ADD,
                                   RL_FUNC_ADD);
}

void set_backface_culling(bool enabled) {
    enabled ? ray::rlEnableBackfaceCulling() : ray::rlDisableBackfaceCulling();
}

void set_cull_face(int face) {
    ray::rlSetCullFace(face);
}

void end_blend_mode() {
    ray::EndBlendMode();
}

void begin_mode_2d(Camera2D camera) {
    ray::BeginMode2D(camera);
}

void end_mode_2d() {
    ray::EndMode2D();
}

void begin_mode_3d(Camera3D camera) {
    ray::BeginMode3D(camera);
}

void end_mode_3d() {
    ray::EndMode3D();
}

Vec2 get_world_to_screen_2d(Vec2 position, Camera2D camera) {
    return ray::GetWorldToScreen2D(position, camera);
}

Vec2 get_screen_to_world_2d(Vec2 position, Camera2D camera) {
    return ray::GetScreenToWorld2D(position, camera);
}

void unload_shader(Shader shader) {
    ray::UnloadShader(shader);
}

bool is_shader_valid(Shader shader) {
    return ray::IsShaderValid(shader);
}

int get_shader_location(Shader shader, const char* name) {
    return ray::GetShaderLocation(shader, name);
}

void set_shader_value(Shader shader, int loc, const void* value, int uniform_type) {
    ray::SetShaderValue(shader, loc, value, uniform_type);
}

void begin_shader_mode(Shader shader) {
    ray::BeginShaderMode(shader);
}

void end_shader_mode() {
    ray::EndShaderMode();
}

Image load_image(const char* path) {
    return to<Image>(ray::LoadImage(path));
}

Image load_image_from_texture(Texture tex) {
    return to<Image>(ray::LoadImageFromTexture(tex));
}

Image load_image_from_screen() {
    return to<Image>(ray::LoadImageFromScreen());
}

void unload_image(Image image) {
    ray::UnloadImage(to<ray::Image>(image));
}

bool export_image(Image image, const char* path) {
    return ray::ExportImage(to<ray::Image>(image), path);
}

Image image_copy(Image image) {
    return to<Image>(ray::ImageCopy(to<ray::Image>(image)));
}

Image image_from_image(Image image, Rect rec) {
    return to<Image>(ray::ImageFromImage(to<ray::Image>(image), rec));
}

void image_format(Image* image, int new_format) {
    ray::ImageFormat(to_ptr<ray::Image>(image), new_format);
}

void image_crop(Image* image, Rect crop) {
    ray::ImageCrop(to_ptr<ray::Image>(image), crop);
}

void image_rotate_cw(Image* image) {
    ray::ImageRotateCW(to_ptr<ray::Image>(image));
}

void image_draw_image_pro(Image* dst, Image src, Rect src_rec, Rect dst_rec, Vec2 origin, float rotation, Color tint) {
    ray::ImageDrawImagePro(to_ptr<ray::Image>(dst), to<ray::Image>(src), src_rec, dst_rec, origin, rotation,
                           {tint.r, tint.g, tint.b, tint.a});
}

Rect get_image_alpha_border(Image image, float threshold) {
    return ray::GetImageAlphaBorder(to<ray::Image>(image), threshold);
}

int get_pixel_data_size(int width, int height, int format) {
    return ray::GetPixelDataSize(width, height, format);
}

void update_texture(Texture tex, const void* pixels) {
    ray::UpdateTexture(tex, pixels);
}

bool is_texture_valid(Texture tex) {
    return ray::IsTextureValid(tex);
}

unsigned char* load_file_data(const char* path, int* size) {
    return ray::LoadFileData(path, size);
}

void unload_file_data(unsigned char* data) {
    ray::UnloadFileData(data);
}

RenderTarget load_render_texture(int width, int height) {
    return ray::LoadRenderTexture(width, height);
}

void unload_render_texture(RenderTarget target) {
    ray::UnloadRenderTexture(target);
}

void begin_texture_mode(RenderTarget target) {
    ray::BeginTextureMode(target);
}

void end_texture_mode() {
    ray::EndTextureMode();
}

Model load_model(const char* path) {
    return to<Model>(ray::LoadModel(path));
}

void unload_model(Model model) {
    ray::UnloadModel(to<ray::Model>(model));
}

ModelAnimation* load_model_animations(const char* path, int* count) {
    return to_ptr<ModelAnimation>(ray::LoadModelAnimations(path, count));
}

void unload_model_animations(ModelAnimation* anims, int count) {
    ray::UnloadModelAnimations(to_ptr<ray::ModelAnimation>(anims), count);
}

void update_model_animation(Model model, ModelAnimation anim, float frame) {
    ray::UpdateModelAnimation(to<ray::Model>(model), to<ray::ModelAnimation>(anim), frame);
}

void draw_mesh(Mesh mesh, Material material, Mat4 transform) {
    ray::DrawMesh(to<ray::Mesh>(mesh), to<ray::Material>(material), transform);
}

void update_mesh_buffer(Mesh mesh, int index, const void* data, int data_size, int offset) {
    ray::UpdateMeshBuffer(to<ray::Mesh>(mesh), index, data, data_size, offset);
}

void set_material_texture(Material* material, int map_type, Texture texture) {
    ray::SetMaterialTexture(to_ptr<ray::Material>(material), map_type, texture);
}

void clear_background(Color color) {
    ray::ClearBackground({color.r, color.g, color.b, color.a});
}

Image gen_image_color(int w, int h, Color color) {
    return to<Image>(ray::GenImageColor(w, h, {color.r, color.g, color.b, color.a}));
}

Shader load_shader(const char* vs_path, const char* fs_path) {
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
