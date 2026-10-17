#include "platform_android.h"

#include <csignal>
#include <filesystem>
#include <spdlog/spdlog.h>
#include <unistd.h>

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
