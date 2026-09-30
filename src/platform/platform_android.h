#pragma once

#ifndef YATAIDON_PLATFORM_ANDROID_H
#define YATAIDON_PLATFORM_ANDROID_H

#include <filesystem>
#include <string>
#include <memory>
#include <atomic>
#include <thread>

// Android-specific platform services.

// Gets the directory for the app's working directory on Android.
// Typically /sdcard/YataiDON or similar.
std::filesystem::path android_get_working_directory();

// Initializes Android-specific crash handlers.
void android_install_crash_handlers();

// Android-specific network services.

// Checks for app updates and installs them if available.
// Runs asynchronously.
void android_check_and_install_update();

// Checks for skin updates from configured repositories.
// Runs asynchronously on a background thread.
void android_check_skin_updates();

// Returns true if a skin update check is currently in progress.
bool android_is_skin_update_in_progress();

// Clean up any Android-specific resources.
void android_cleanup();

#endif // YATAIDON_PLATFORM_ANDROID_H