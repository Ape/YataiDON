#pragma once

#ifndef YATAIDON_PLATFORM_UNIX_H
#define YATAIDON_PLATFORM_UNIX_H

#include <filesystem>
#include <string>
#include <unordered_set>
#include <mutex>

// Unix/Linux-specific platform services.

// Gets the directory containing the current executable.
// Works on Linux (via /proc/self/exe) and other Unix systems.
std::filesystem::path unix_get_executable_dir();

// Initializes Unix-specific crash handlers (signal handlers with altstack for stack overflow protection).
void unix_install_crash_handlers();

// Linux-specific text input handling for IME/composition.
// Returns true if the event was handled.
bool linux_handle_text_input(void* event, bool input_locked,
                              std::mutex& input_mutex,
                              std::unordered_multiset<int>& pressed_keys,
                              std::unordered_multiset<int>& released_keys);

#endif // YATAIDON_PLATFORM_UNIX_H
