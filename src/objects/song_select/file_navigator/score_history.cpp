#include "score_history.h"
#include "../../../libs/text.h"

namespace {
    std::unique_ptr<OutlinedText> build_leaderboard_title(const SkinInfo& box_cfg) {
        const std::string& lang = global_data.config->general.language;
        const SkinInfo& cfg = tex.skin_config[SC::LEADERBOARD_TITLE];
        auto it = cfg.text.find(lang);
        std::string label = it != cfg.text.end() ? it->second
                           : !cfg.text.empty()   ? cfg.text.begin()->second
                                                  : "";
        int font_size = (int)box_cfg.height;
        while (font_size > 8) {
            float w = ray::MeasureTextEx(font_manager.get_font(label, font_size), label.c_str(), (float)font_size, 2.0f).x;
            if (w <= box_cfg.width) break;
            font_size--;
        }
        return std::make_unique<OutlinedText>(label, font_size, ray::BLACK, ray::BLANK, false);
    }

    OutlinedText* leaderboard_title_long() {
        static std::unique_ptr<OutlinedText> cache;
        static std::string cached_lang;
        const std::string& lang = global_data.config->general.language;
        if (lang != cached_lang) {
            cache = build_leaderboard_title(tex.skin_config[SC::LEADERBOARD_TITLE_LONG]);
            cached_lang = lang;
        }
        return cache.get();
    }

}

ScoreHistory::ScoreHistory(const std::array<std::optional<Score>, 5>& scores, double current_ms)
    : last_ms(current_ms)
{
    for (int i = 0; i < 5; i++) {
        if (scores[i].has_value())
            available.push_back({i, scores[i].value()});
    }
    if (!available.empty())
        curr_index = 1 % (int)available.size();

    t_background_2 = tex.get_texture("leaderboard/background_2");
    t_shinuchi_ura = tex.get_texture("leaderboard/shinuchi_ura");
    t_shinuchi = tex.get_texture("leaderboard/shinuchi");
    t_pts = tex.get_texture("leaderboard/pts");
    t_normal = tex.get_texture("leaderboard/normal");
    t_difficulty = tex.get_texture("leaderboard/difficulty");
    t_judge_good = tex.get_texture("leaderboard/judge_good");
    t_judge_ok = tex.get_texture("leaderboard/judge_ok");
    t_judge_bad = tex.get_texture("leaderboard/judge_bad");
    t_judge_drumroll = tex.get_texture("leaderboard/judge_drumroll");
    t_counter = tex.get_texture("leaderboard/counter");
    t_judge_num = tex.get_texture("leaderboard/judge_num");
}

void ScoreHistory::update(double current_ms) {
    if (available.empty()) return;
    if (current_ms >= last_ms + 1000.0) {
        last_ms = current_ms;
        curr_index = (curr_index + 1) % (int)available.size();
    }
}

void ScoreHistory::draw() {
    if (available.empty()) return;
    draw_long();
}

void ScoreHistory::draw_long() {
    const auto& [curr_diff, score] = available[curr_index];
    const std::string& score_method = global_data.config->general.score_method;
    float offset_y = tex.skin_config[SC::SCORE_INFO_BG_OFFSET].y;
    float margin_w = tex.skin_config[SC::SCORE_INFO_COUNTER_MARGIN].width;
    float margin_x = tex.skin_config[SC::SCORE_INFO_COUNTER_MARGIN].x;

    tex.draw_texture(t_background_2, {});
    const SkinInfo& title_pos = tex.skin_config[SC::LEADERBOARD_TITLE_LONG];
    leaderboard_title_long()->draw({.x = title_pos.x, .y = title_pos.y});

    if (score_method == ScoreMethod::SHINUCHI) {
        if (curr_diff == (int)Difficulty::URA)
            tex.draw_texture(t_shinuchi_ura, {.index = 1});
        else
            tex.draw_texture(t_shinuchi, {.index = 1});
        tex.draw_texture(t_pts, {.color = ray::WHITE, .index = 1});
    } else {
        tex.draw_texture(t_normal, {.index = 1});
        tex.draw_texture(t_pts, {.color = ray::BLACK, .index = 1});
    }

    tex.draw_texture(t_difficulty, {.frame = curr_diff, .index = 1});

    for (int i = 0; i < 4; i++)
        tex.draw_texture(t_normal, {.y = offset_y + i * offset_y, .index = 1});

    tex.draw_texture(t_judge_good, {});
    tex.draw_texture(t_judge_ok, {});
    tex.draw_texture(t_judge_bad, {});
    tex.draw_texture(t_judge_drumroll, {});

    std::array<int, 5> values = {score.score, score.good, score.ok, score.bad, score.drumroll};
    ray::Color score_color = (score_method == ScoreMethod::SHINUCHI) ? ray::WHITE : ray::BLACK;

    for (int j = 0; j < 5; j++) {
        std::string counter = std::to_string(values[j]);
        int len = (int)counter.size();
        if (j == 0) {
            for (int i = 0; i < len; i++) {
                float x = -((len * margin_w) / 2.0f) + (i * margin_w);
                tex.draw_texture(t_counter, {.color = score_color, .frame = counter[i] - '0', .x = x, .index = 1});
            }
        } else {
            for (int i = 0; i < len; i++) {
                float x = -(float)(len - i) * margin_x;
                float y = (float)j * offset_y;
                tex.draw_texture(t_judge_num, {.frame = counter[i] - '0', .x = x, .y = y});
            }
        }
    }
}
