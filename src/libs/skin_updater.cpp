#include "skin_updater.h"

#include <cctype>
#include <cstdint>
#include <fstream>
#include <sstream>

#include <spdlog/spdlog.h>

#include "sha256.h"

#ifndef __EMSCRIPTEN__

#include <cpr/cpr.h>

#if defined(__ANDROID__)
// Bundled Mozilla CA bundle for libcurl on Android (which has no system trust
// store). Defined in libs/network.cpp; reused here so skin updates trust the
// same roots as the rest of the client.
cpr::SslOptions android_ca();
#include <curl/curl.h>
#endif

namespace fs = std::filesystem;

namespace {

std::string strip_dot_git(std::string url) {
    if (url.size() >= 4 && url.compare(url.size() - 4, 4, ".git") == 0) url.resize(url.size() - 4);
    return url;
}

void trim(std::string& s) {
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) s.pop_back();
}

cpr::Response get_url(const std::string& url, int32_t timeout_ms, int32_t connect_timeout_ms) {
    cpr::Session session;
    session.SetUrl(cpr::Url{url});
    session.SetTimeout(cpr::Timeout{timeout_ms});
    session.SetConnectTimeout(cpr::ConnectTimeout{connect_timeout_ms});
#if defined(__ANDROID__)
    session.SetOption(android_ca());
    curl_easy_setopt(session.GetCurlHolder()->handle, CURLOPT_SSL_EC_CURVES, "X25519:P-256:P-384");
#endif
    return session.Get();
}

}  // namespace

void SkinUpdater::set_current_skin(const std::string& name) {
    std::lock_guard<std::mutex> lock(status_mutex_);
    current_skin_ = name;
    current_file_.clear();
}

void SkinUpdater::set_current_file(const std::string& file) {
    std::lock_guard<std::mutex> lock(status_mutex_);
    current_file_ = file;
}

void SkinUpdater::note_file_updated() {
    std::lock_guard<std::mutex> lock(status_mutex_);
    ++files_updated_;
}

void SkinUpdater::update_one_skin(const std::filesystem::path& skin_dir, const std::string& repo_url, const std::string& branch) {
    set_current_skin(skin_dir.filename().string());

    auto raw_url = [&](const std::string& rel_path) {
        return repo_url + "/raw/branch/" + branch + "/" + rel_path;
    };

    cpr::Response checksums = get_url(raw_url("checksums.sha256"), 10000, 5000);
    if (checksums.status_code != 200) {
        spdlog::warn("Skin update ({}): could not fetch checksums.sha256 (HTTP {}, curl error {}: {})",
                     skin_dir.filename().string(), checksums.status_code,
                     static_cast<int>(checksums.error.code), checksums.error.message);
        return;
    }

    std::istringstream lines(checksums.text);
    std::string hash, rel_path;
    int updated = 0;
    while (lines >> hash >> rel_path) {
        if (!rel_path.empty() && rel_path.front() == '*') rel_path.erase(0, 1);
        if (fs::path(rel_path).filename() == "checksums.sha256") continue;

        fs::path local_file = (skin_dir / rel_path).lexically_normal();
        const fs::path local_rel = local_file.lexically_relative(skin_dir);
        if (local_rel.empty() || *local_rel.begin() == "..") {
            spdlog::warn("Skin update ({}): skipping unsafe manifest path {}", skin_dir.filename().string(), rel_path);
            continue;
        }
        std::error_code size_ec;
        uintmax_t size = fs::file_size(local_file, size_ec);
        if (!size_ec) {
            std::ifstream in(local_file, std::ios::binary);
            std::string contents(size, '\0');
            in.read(contents.data(), static_cast<std::streamsize>(size));
            if (crypto::to_hex(crypto::sha256(contents)) == hash) continue;
        }

        // File is missing or differs: this pass has at least one real update.
        any_updated_.store(true);
        set_current_file(rel_path);
        cpr::Response file_resp = get_url(raw_url(rel_path), 15000, 5000);
        if (file_resp.status_code != 200) {
            spdlog::warn("Skin update ({}): failed to download {} (HTTP {})", skin_dir.filename().string(), rel_path, file_resp.status_code);
            continue;
        }
        std::error_code mkdir_ec;
        fs::create_directories(local_file.parent_path(), mkdir_ec);
        std::ofstream out(local_file, std::ios::binary | std::ios::trunc);
        if (!out) {
            spdlog::warn("Skin update ({}): failed to write {}", skin_dir.filename().string(), rel_path);
            continue;
        }
        out << file_resp.text;
        note_file_updated();
        ++updated;
    }
    spdlog::info("Skin update ({}): {} file(s) updated", skin_dir.filename().string(), updated);
}

void SkinUpdater::scan_skins() {
    std::error_code ec;
    if (!fs::exists("Skins", ec)) return;
    for (const auto& entry : fs::directory_iterator("Skins", ec)) {
        if (ec || !entry.is_directory()) continue;

        std::ifstream repo_file(entry.path() / ".skin-repo");
        if (!repo_file) continue;
        std::string repo_url, branch;
        std::getline(repo_file, repo_url);
        std::getline(repo_file, branch);
        trim(repo_url);
        trim(branch);
        if (repo_url.empty()) continue;
        if (branch.empty()) branch = "main";

        update_one_skin(entry.path(), strip_dot_git(repo_url), branch);
    }
}

void SkinUpdater::start() {
    if (started_.exchange(true)) return;
    running_ = true;
    thread_ = std::thread([this] {
        try {
            scan_skins();
        } catch (const std::exception& e) {
            spdlog::error("Skin update: unhandled exception: {}", e.what());
        }
        running_ = false;
        std::lock_guard<std::mutex> lock(status_mutex_);
        current_skin_.clear();
        current_file_.clear();
    });
}

bool SkinUpdater::in_progress() {
    return running_.load();
}

bool SkinUpdater::finished() {
    return started_.load() && !running_.load();
}

bool SkinUpdater::applying_updates() {
    return running_.load() && any_updated_.load();
}

bool SkinUpdater::updated_this_pass() {
    return any_updated_.load();
}

SkinUpdater::Status SkinUpdater::snapshot() {
    Status s;
    s.running = running_.load();
    s.finished = finished();
    s.any_updated = any_updated_.load();
    std::lock_guard<std::mutex> lock(status_mutex_);
    s.current_skin = current_skin_;
    s.current_file = current_file_;
    s.files_updated = files_updated_;
    return s;
}

void SkinUpdater::shutdown() {
    if (thread_.joinable()) thread_.join();
}

SkinUpdater skin_updater;

#else  // __EMSCRIPTEN__

void SkinUpdater::start() {}
bool SkinUpdater::in_progress() { return false; }
bool SkinUpdater::finished() { return true; }
bool SkinUpdater::applying_updates() { return false; }
bool SkinUpdater::updated_this_pass() { return false; }
SkinUpdater::Status SkinUpdater::snapshot() {
    Status s;
    s.finished = true;
    return s;
}
void SkinUpdater::shutdown() {}

SkinUpdater skin_updater;

#endif
