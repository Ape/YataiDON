#pragma once

#include <filesystem>
#include "nus3bank.h"

namespace fs = std::filesystem;

namespace gen3 {

bool decode_nub(const fs::path& path, gen4::DecodedAudio& out);

}  // namespace gen3
