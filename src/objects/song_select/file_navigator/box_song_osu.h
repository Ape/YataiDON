#pragma once

#include "../../../libs/filesystem.h"
#include "box_song.h"

class SongBoxOsu : public SongBox {
public:
    SongBoxOsu(const fs::path& path, const BoxDef& box_def, SongParser parser);
};
