#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace gen3 {

struct SongEntry {
    std::string id;
    int         unique_id = 0;
    std::string title;
    std::string genre;
    bool        secret = false;
    int         stars[5] = {};
    bool        has_chart[5] = {};
    bool        charts_known = false;
};

class Library {
public:
    // On success, `data_root`/`entries`/`name` reflect the loaded config.
    // On failure (returns false), any of those may hold partially-updated
    // state from this attempt; check loaded() before trusting other accessors.
    bool load(const fs::path& data_root);

    bool loaded() const { return is_loaded; }
    const fs::path& root() const { return data_root; }

    const std::string& game_name() const { return name; }

    const SongEntry* find(const std::string& id) const;
    const std::vector<SongEntry>& songs() const { return entries; }

    // `difficulty` is a 0-based index into the 5-entry stars[]/has_chart[]
    // arrays (easy, normal, hard, oni/master, ura). Out of [0,4] returns an
    // empty path / false.
    fs::path chart_path(const std::string& id, int difficulty) const;
    bool has_difficulty(const std::string& id, int difficulty) const;

    fs::path sound_path(const std::string& id) const;

private:
    bool                   is_loaded = false;
    fs::path               data_root;
    std::string            name;
    std::vector<SongEntry> entries;

    void load_tuning(const fs::path& path);
};

std::vector<std::string> genres_present(const Library& library);

int genre_index_for(const std::string& genre);

fs::path    genre_path(const fs::path& data_root, const std::string& genre);
std::string genre_of_path(const fs::path& path);

// Returns a pointer into a process-lifetime cache keyed by data root; once a
// root is cached the Library is never mutated again, so the pointer stays
// valid for the life of the program (or nullptr if the root can't be loaded).
const Library* library_for(const fs::path& path);

fs::path find_data_root(const fs::path& path);

}  // namespace gen3
