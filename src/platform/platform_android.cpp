#include "platform_android.h"

#include <spdlog/spdlog.h>
#include <filesystem>
#include <memory>
#include <atomic>
#include <thread>
#include <chrono>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <cctype>
#include <mutex>
#include <csignal>

#include <cpr/cpr.h>

#include <openssl/crypto.h>
#include "libs/filesystem.h"
#include "libs/sha256.h"

namespace {

// Android-specific SSL options
cpr::SslOptions g_android_ssl_options;

std::string strip_dot_git(std::string url) {
    if (url.size() >= 4 && url.compare(url.size() - 4, 4, ".git") == 0) url.resize(url.size() - 4);
    return url;
}

void trim(std::string& s) {
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) s.pop_back();
}

cpr::Response get_classical_tls(const std::string& url, int32_t timeout_ms, int32_t connect_timeout_ms) {
    cpr::Session session;
    session.SetUrl(cpr::Url{url});
    session.SetTimeout(cpr::Timeout{timeout_ms});
    session.SetConnectTimeout(cpr::ConnectTimeout{connect_timeout_ms});
    session.SetOption(g_android_ssl_options);
    curl_easy_setopt(session.GetCurlHolder()->handle, CURLOPT_SSL_EC_CURVES, "X25519:P-256:P-384");
    return session.Get();
}

void update_one_skin(const std::filesystem::path& skin_dir, const std::string& repo_url, const std::string& branch) {
    auto raw_url = [&](const std::string& rel_path) {
        return repo_url + "/raw/branch/" + branch + "/" + rel_path;
    };

    cpr::Response checksums = get_classical_tls(raw_url("checksums.sha256"), 10000, 5000);
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
        if (std::filesystem::path(rel_path).filename() == "checksums.sha256") continue;

        std::filesystem::path local_file = (skin_dir / rel_path).lexically_normal();
        const std::filesystem::path local_rel = local_file.lexically_relative(skin_dir);
        if (local_rel.empty() || *local_rel.begin() == "..") {
            spdlog::warn("Skin update ({}): skipping unsafe manifest path {}", skin_dir.filename().string(), rel_path);
            continue;
        }
        std::error_code size_ec;
        uintmax_t size = std::filesystem::file_size(local_file, size_ec);
        if (!size_ec) {
            std::ifstream in(local_file, std::ios::binary);
            std::string contents(size, '\0');
            in.read(contents.data(), static_cast<std::streamsize>(size));
            if (crypto::to_hex(crypto::sha256(contents)) == hash) continue;
        }

        cpr::Response file_resp = get_classical_tls(raw_url(rel_path), 15000, 5000);
        if (file_resp.status_code != 200) {
            spdlog::warn("Skin update ({}): failed to download {} (HTTP {})", skin_dir.filename().string(), rel_path, file_resp.status_code);
            continue;
        }
        std::error_code mkdir_ec;
        std::filesystem::create_directories(local_file.parent_path(), mkdir_ec);
        std::ofstream out(local_file, std::ios::binary | std::ios::trunc);
        if (!out) {
            spdlog::warn("Skin update ({}): failed to write {}", skin_dir.filename().string(), rel_path);
            continue;
        }
        out << file_resp.text;
        ++updated;
    }
    spdlog::info("Skin update ({}): {} file(s) updated", skin_dir.filename().string(), updated);
}

void scan_skins() {
    std::error_code ec;
    if (!std::filesystem::exists("Skins", ec)) return;
    for (const auto& entry : std::filesystem::directory_iterator("Skins", ec)) {
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

} // namespace

// Static storage for async operations
static std::optional<cpr::AsyncResponse> g_pending_update_checksum;
static bool g_android_update_checked = false;
static std::thread g_skin_update_thread;
static std::shared_ptr<std::atomic<bool>> g_skin_update_done = std::make_shared<std::atomic<bool>>(false);
static std::mutex g_skin_update_mutex;

std::filesystem::path android_get_working_directory() {
    std::filesystem::path exe_dir("/sdcard/YataiDON");
    std::error_code ec;
    std::filesystem::create_directories(exe_dir, ec);
    std::filesystem::current_path(exe_dir, ec);
    if (ec) {
        spdlog::error("Failed to set working directory to {}: {}", exe_dir.string(), ec.message());
    } else {
        spdlog::info("Working directory set to: {}", exe_dir.string());
    }
    return exe_dir;
}

void android_install_crash_handlers() {
    // Android uses its own crash handling (tombstones, logcat)
    // We just set up basic signal handlers
    signal(SIGINT, [](int) { _exit(0); });
}

void android_check_and_install_update() {
    if (g_android_update_checked) return;
    g_android_update_checked = true;

    constexpr char kUpdateChecksumUrl[] =
        "https://github.com/yonokid/YataiDON/releases/latest/download/checksums-android.sha256";

    g_pending_update_checksum = cpr::GetAsync(cpr::Url{kUpdateChecksumUrl}, cpr::Timeout{5000});
}

void android_check_skin_updates() {
    std::lock_guard<std::mutex> lock(g_skin_update_mutex);
    if (g_skin_update_thread.joinable()) return;

    g_skin_update_done = std::make_shared<std::atomic<bool>>(false);
    std::shared_ptr<std::atomic<bool>> done = g_skin_update_done;

    g_skin_update_thread = std::thread([done] {
        scan_skins();
        done->store(true);
    });
}

bool android_is_skin_update_in_progress() {
    std::lock_guard<std::mutex> lock(g_skin_update_mutex);
    if (!g_skin_update_done) return false;
    return !g_skin_update_done->load();
}

struct AndroidSSLOptionsImpl {
    cpr::SslOptions options;
};

struct AndroidSSLOptions android_get_ssl_options() {
    // Initialize Android CA bundle
    static AndroidSSLOptionsImpl impl;
    static bool initialized = false;

    if (!initialized) {
        // Android CA certificate bundle path
        const char* ca_bundle_path = "/system/etc/security/cacerts";
        if (std::filesystem::exists(ca_bundle_path)) {
            impl.options = cpr::Ssl(cpr::ssl::CaPath(ca_bundle_path));
        }
        initialized = true;
    }

    AndroidSSLOptions result;
    result.options = &impl.options;
    return result;
}

void android_cleanup() {
    if (g_skin_update_thread.joinable()) {
        g_skin_update_thread.join();
    }
}
