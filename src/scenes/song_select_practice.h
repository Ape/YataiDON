#pragma once
#include "../objects/global/practice_timer.h"
#include "song_select.h"

class PracticeSongSelectScreen : public SongSelectScreen {
protected:
    Screens get_game_screen_target() override { return Screens::GAME_PRACTICE; }
    bool hides_dan() override { return true; }
    bool allows_second_player_join() override { return false; }

public:
    PracticeSongSelectScreen() : SongSelectScreen("song_select") {}

    bool uses_session_timer() const override { return true; }
    void on_screen_start() override {
        SongSelectScreen::on_screen_start();
        PracticeTimer::begin_session(get_frame_ms());
    }
    Screens on_screen_end(Screens next_screen) override {
        if (next_screen != Screens::GAME_PRACTICE && next_screen != Screens::PRACTICE_SELECT) {
            PracticeTimer::end_session();
        }
        return SongSelectScreen::on_screen_end(next_screen);
    }
};
