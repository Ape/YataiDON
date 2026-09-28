#pragma once

#ifndef YATAIDON_PLATFORM_MACOS_H
#define YATAIDON_PLATFORM_MACOS_H

#include <filesystem>

// macOS-specific platform services (for desktop macOS, not iOS).

// Gets the directory containing the current executable.
// Uses _NSGetExecutablePath and realpath.
std::filesystem::path macos_get_executable_dir();

// Initializes macOS-specific crash handlers.
void macos_install_crash_handlers();

#endif // YATAIDON_PLATFORM_MACOS_H