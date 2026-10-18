#pragma once

#include <algorithm>
#include <cmath>
#include <optional>

class PlaybackClock {
public:
    double position() const { return position_ms; }
    bool is_paused() const { return paused; }

    void reset(double current_ms, double chart_ms = 0.0) {
        paused = false;
        seek(current_ms, chart_ms);
    }

    void update(double current_ms) {
        if (!paused) {
            position_ms = current_ms - origin_ms;
        }
    }

    void pause(double current_ms) {
        if (paused) {
            return;
        }

        update(current_ms);
        paused = true;
    }

    void resume(double current_ms) {
        if (!paused) {
            return;
        }

        paused = false;
        seek(current_ms, position_ms);
    }

    void seek(double current_ms, double chart_ms) {
        position_ms = chart_ms;
        origin_ms = current_ms - chart_ms;
        sync = {};
    }

    void follow_audio(double current_ms, double chart_ms) {
        if (paused) {
            return;
        }

        position_ms = chart_ms;
        origin_ms = current_ms - chart_ms;
    }

    template<typename RefreshRateFn>
    void synchronize(double current_ms, double audio_ms_adjusted, RefreshRateFn&& get_refresh_hz) {
        if (paused) {
            return;
        }

        // Advance the chart by whole display refreshes when the frame took about that long (a fixed
        // per-frame step): with vsync the frame-start timestamps jitter by a few tenths
        // of a ms around the refresh period, and only the notes would carry that jitter. Frames far
        // from a whole number of refreshes (no vsync, a limiter below the refresh rate) keep the
        // measured time; the loop below absorbs any rate error against the audio.
        if (!sync.previous_ms) {
            const int refresh_hz = get_refresh_hz();
            sync.refresh_period_ms = refresh_hz > 0 ? 1000.0 / refresh_hz : 0.0;
        } else if (sync.refresh_period_ms > 0.0) {
            double advance = position_ms - sync.chart_ms;
            double refreshes = std::round(advance / sync.refresh_period_ms);

            if (refreshes >= 1.0 &&
                std::abs(advance - refreshes * sync.refresh_period_ms) < 0.25 * sync.refresh_period_ms) {
                position_ms = sync.chart_ms + refreshes * sync.refresh_period_ms;
            }
        }

        double drift = audio_ms_adjusted - position_ms;
        if (std::abs(drift) > 100.0) {
            position_ms = audio_ms_adjusted;
            sync.drift_ema = 0.0;
            sync.rate_bias = 0.0;
        } else {
            // The audio position advances in device-buffer steps, on some devices in irregular bursts
            // (several ms of noise), and the audio clock runs slightly off the system clock. Jumping
            // the chart onto it whenever the drift passed 5 ms moved only the notes (everything else
            // runs on the frame clock), about a 1.5 ms hitch every second or so. Instead the drift is
            // smoothed and the chart clock's rate is nudged toward it (a PI loop: the integral learns
            // the audio clock's rate offset, which is not always small -- 0.1-0.8% on a virtual
            // output device). At most 2%, 20% while it is more than 20 ms off (after a stall).
            double dt = sync.previous_ms ? std::clamp(current_ms - *sync.previous_ms, 0.0, 100.0) : 0.0;
            if (!sync.previous_ms) {
                sync.drift_ema = drift;
                sync.rate_bias = 0.0;
            } else {
                sync.drift_ema += (drift - sync.drift_ema) * std::min(1.0, dt / 250.0);
            }

            sync.rate_bias = std::clamp(sync.rate_bias + sync.drift_ema * dt / (500.0 * 2000.0), -0.02, 0.02);
            double max_rate = std::abs(sync.drift_ema) > 20.0 ? 0.2 : 0.02;
            double step = std::clamp((sync.rate_bias + sync.drift_ema / 500.0) * dt, -max_rate * dt, max_rate * dt);
            position_ms += step;
            sync.drift_ema -= step;
        }

        sync.previous_ms = current_ms;
        sync.chart_ms = position_ms;
        origin_ms = current_ms - position_ms;
    }

private:
    struct SyncState {
        std::optional<double> previous_ms;
        double chart_ms = 0.0;
        double drift_ema = 0.0;
        double rate_bias = 0.0;
        double refresh_period_ms = 0.0;
    };

    double position_ms = 0.0;
    double origin_ms = 0.0;
    bool paused = false;
    SyncState sync;
};
