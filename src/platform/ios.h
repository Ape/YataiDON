#pragma once

// iOS-only platform services. Paths are resolved anew on every launch because
// installing an update can relocate both the bundle and the data container.
//
// Prepares the sandboxed Documents directory and copies bundled game data into
// it (skipping files that already exist, except shaders which are always
// refreshed to match the running binary), then chdirs into it. Throws
// std::runtime_error if the Documents directory cannot be located; other
// per-file copy failures are logged and skipped rather than fatal.
void ios_prepare_filesystem();

// Monotonic (steady_clock-based) milliseconds since an arbitrary epoch, frozen
// while ios_set_suspended(true) is in effect. Safe to call from any thread.
double ios_game_time_ms();

// Suspend/resume state shared with ios_game_time_ms()/ios_is_suspended(); all
// three are internally mutex-guarded, so calling them from different threads
// (e.g. the app lifecycle thread vs. the game/audio thread) is safe.
void ios_set_suspended(bool suspended);
bool ios_is_suspended();

// Requests a low-latency preferred I/O buffer duration from AVAudioSession.
// This does not hand back or take ownership of any audio buffer - it only
// sets a latency hint on the shared AVAudioSession and returns synchronously;
// failures are logged (via spdlog::warn) and otherwise ignored.
void ios_request_audio_buffer();
