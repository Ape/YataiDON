#pragma once

#include "box_folder.h"

class GenreBG {
private:
    ray::Shader shader;
    bool shader_loaded = false;
    std::unique_ptr<OutlinedText> name;
    TextureIndex texture_index;
    void draw_anim(FolderBox* box);
    void draw_exit_anim(float start_position, float end_position, FolderBox* folder);

    std::unique_ptr<MoveAnimation> stretch;
    std::unique_ptr<TextureResizeAnimation> scale;
    std::unique_ptr<MoveAnimation> move;
    std::unique_ptr<FadeAnimation> fade;
    std::unique_ptr<MoveAnimation> move_left;
    std::unique_ptr<MoveAnimation> move_right;

    // Fixed-path textures resolved once in the constructor instead of calling
    // tex.get_texture() every frame from draw()/draw_anim()/draw_exit_anim().
    TextureObject* t_folder_background_edge = nullptr;
    TextureObject* t_folder_background = nullptr;
    TextureObject* t_folder_background_folder_edge = nullptr;
    TextureObject* t_folder_background_folder = nullptr;
public:
    GenreBG(const std::string& text_name, std::optional<ray::Color> color, TextureIndex texture_index, float distance);
    ~GenreBG() {
        if (shader_loaded) ray::UnloadShader(shader);
    }
    void update(double current_ms, FolderBox* box);
    void draw(float start_position, float end_position, FolderBox* folder);
    void exit(float left_position, float right_position, FolderBox* center_box);
    void fade_out();
    void fade_in();

    bool is_finished();
    bool is_complete();
};
