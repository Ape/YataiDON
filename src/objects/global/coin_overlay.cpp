#include "coin_overlay.h"

#include "../../libs/global_data.h"

namespace {
    class PlayerNumGuard {
    public:
        explicit PlayerNumGuard(PlayerNum player_num) : previous_(global_data.player_num) {
            global_data.player_num = player_num;
        }

        ~PlayerNumGuard() { global_data.player_num = previous_; }

    private:
        PlayerNum previous_;
    };
} // namespace

CoinOverlay::CoinOverlay() {
    if (!load("CoinOverlay", "coin_overlay")) {
        return;
    }
    fn_update = lua_object["update"];
    fn_draw = lua_object["draw"];
}

void CoinOverlay::update(double current_ms) {
    call(fn_update, "CoinOverlay:update", current_ms);
}
void CoinOverlay::draw(float x, float y, bool suppress_second_player_prompt) {
    if (!suppress_second_player_prompt) {
        call(fn_draw, "CoinOverlay:draw", x, y);
        return;
    }

    // CoinOverlay's skin script treats TWO_PLAYER as having no open join prompt.
    PlayerNumGuard player_num_guard(PlayerNum::TWO_PLAYER);
    call(fn_draw, "CoinOverlay:draw", x, y);
}
