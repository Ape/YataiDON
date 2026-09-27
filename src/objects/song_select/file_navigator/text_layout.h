#pragma once

#include "../../../libs/text.h"
#include <string>
#include <unordered_set>
#include <vector>

inline bool language_is_cjk(const std::string& lang) {
    static const std::unordered_set<std::string> cjk = {
        "ja", "zh", "ko", "zh_tw", "zh-tw", "zh_cn", "zh-cn",
    };
    return cjk.count(lang) > 0;
}

inline std::string word_wrap(const std::string& text, int font_size, float spacing, float max_width) {
    ray::Font font = font_manager.get_font(text, font_size);

    std::vector<std::string> words;
    size_t start = 0;
    while (start < text.size()) {
        size_t sp = text.find(' ', start);
        if (sp == std::string::npos) { words.push_back(text.substr(start)); break; }
        words.push_back(text.substr(start, sp - start));
        start = sp + 1;
    }

    // Splits a single space-free token into UTF-8-safe pieces no wider than
    // max_width, so a long word/URL/CJK run gets hard-broken instead of
    // overflowing the box.
    auto break_long_word = [&](const std::string& word) {
        std::vector<std::string> pieces;
        std::string piece;
        size_t i = 0;
        while (i < word.size()) {
            size_t char_len = 1;
            while (i + char_len < word.size() && ((unsigned char)word[i + char_len] & 0xC0) == 0x80)
                char_len++;
            std::string candidate = piece + word.substr(i, char_len);
            if (!piece.empty() && ray::MeasureTextEx(font, candidate.c_str(), (float)font_size, spacing).x > max_width) {
                pieces.push_back(piece);
                piece = word.substr(i, char_len);
            } else {
                piece = candidate;
            }
            i += char_len;
        }
        if (!piece.empty()) pieces.push_back(piece);
        return pieces;
    };

    std::string wrapped, current;
    for (const auto& w : words) {
        std::string candidate = current.empty() ? w : current + " " + w;
        float width = ray::MeasureTextEx(font, candidate.c_str(), (float)font_size, spacing).x;
        if (width <= max_width) {
            current = candidate;
            continue;
        }
        if (!current.empty()) {
            wrapped += current + "\n";
            current.clear();
        }
        if (ray::MeasureTextEx(font, w.c_str(), (float)font_size, spacing).x > max_width) {
            std::vector<std::string> pieces = break_long_word(w);
            for (size_t i = 0; i + 1 < pieces.size(); i++) wrapped += pieces[i] + "\n";
            current = pieces.empty() ? "" : pieces.back();
        } else {
            current = w;
        }
    }
    if (!current.empty()) wrapped += current;
    return wrapped;
}
