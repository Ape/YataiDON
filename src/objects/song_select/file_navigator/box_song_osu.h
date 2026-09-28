#pragma once

#include "box_song.h"
#include "../../../libs/filesystem.h"

class SongBoxOsu : public SongBox {
public:
    SongBoxOsu(const fs::path& path, const BoxDef& box_def, SongParser parser);

protected:
    void draw_closed() override;
    void draw_open() override;
    void load_textures() override;

    TextureObject* t_favorite_1p = nullptr;
    TextureObject* t_favorite_2p = nullptr;
};
