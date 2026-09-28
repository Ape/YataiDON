#pragma once

#include <cstdint>
#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace gen4 {

std::vector<uint8_t> derive_key(const std::string& seed);

std::vector<uint8_t> load_encrypted(const fs::path& path, const std::vector<uint8_t>& key);

bool looks_encrypted(const std::vector<uint8_t>& raw);

struct SongEntry {
    std::string id;
    int         unique_id = 0;
    int         genre_no  = 0;
    std::string sound_file;
    int         stars[5]  = {};
    bool        branch[5] = {};
    std::map<std::string, std::string> title;
    std::map<std::string, std::string> subtitle;
};

struct OrderEntry {
    int         genre_no = 0;
    std::string id;
};

class Library {
public:
    // Check loaded() before trusting songs()/order()/root(): a failed load()
    // (returns false) may leave those holding partial state from the attempt.
    bool load(const fs::path& data_root);

    bool loaded() const { return is_loaded; }
    const fs::path& root() const { return data_root; }

    // The returned pointer is valid only as long as this Library is not
    // reloaded (load() clears and repopulates `entries`); it is not
    // synchronized against a concurrent load() on the same instance.
    const SongEntry* find(const std::string& id) const;
    const std::vector<SongEntry>& songs() const { return entries; }

    const std::vector<OrderEntry>& order() const { return order_entries; }

    std::vector<uint8_t> load_chart(const std::string& id, int difficulty) const;

    bool has_difficulty(const std::string& id, int difficulty) const;

private:
    bool                   is_loaded = false;
    fs::path               data_root;
    std::vector<SongEntry>  entries;
    std::vector<OrderEntry> order_entries;
    std::vector<uint8_t>    fumen_key;
};

int genre_index_for(int genre_no);

std::string genre_name(int genre_no, const std::string& language);

std::vector<int> genres_present(const Library& library);

fs::path genre_path(const fs::path& data_root, int genre_no);

int genre_of_path(const fs::path& path);

// Returns a pointer into a process-lifetime cache keyed by data root; once a
// root is cached the Library is never mutated again, so the pointer stays
// valid for the life of the program (or nullptr if the root can't be loaded).
const Library* library_for(const fs::path& path);

fs::path find_data_root(const fs::path& path);

std::vector<uint8_t> sha256(const uint8_t* data, size_t len);
std::vector<uint8_t> aes256_cbc_decrypt(const std::vector<uint8_t>& key,
                                        const uint8_t* iv,
                                        const uint8_t* data, size_t len);
std::vector<uint8_t> gzip_inflate(const uint8_t* data, size_t len);

}  // namespace gen4
