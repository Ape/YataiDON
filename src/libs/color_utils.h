#pragma once

#include <cstdint>
#include <concepts>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <type_traits>

struct Color {
    uint8_t r = 0, g = 0, b = 0, a = 0;

    constexpr Color() = default;
    constexpr Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0) : r(r), g(g), b(b), a(a) {};
    constexpr bool operator==(const Color&) const = default;

    // Implicit interop with raylib's ray::Color (and its WHITE/BLACK/... macros), without including ray.h.
    template <class T>
        requires(!std::same_as<T, Color> && requires(const T& t) { t.r; t.g; t.b; t.a; })
    constexpr Color(const T& c) : r(c.r), g(c.g), b(c.b), a(c.a) {}

    template <class T>
        requires(!std::same_as<T, Color> && std::is_aggregate_v<T> && sizeof(T) == 4 &&
                 requires(T t) { t.r; t.g; t.b; t.a; })
    constexpr operator T() const {
        return T{r, g, b, a};
    }
};

inline Color parse_hex_color(const std::string& input) {
    std::string color = input;
    if (!color.empty() && color[0] == '#') {
        color = color.substr(1);
    }
    if (color.size() == 3) {
        color = std::string{color[0], color[0], color[1], color[1], color[2], color[2]};
    }
    if (color.size() != 6) {
        throw std::invalid_argument("Invalid hex color: " + input);
    }

    auto nibble = [&](char c) -> int {
        if (c >= '0' && c <= '9') {
            return c - '0';
        }
        if (c >= 'a' && c <= 'f') {
            return c - 'a' + 10;
        }
        if (c >= 'A' && c <= 'F') {
            return c - 'A' + 10;
        }
        throw std::invalid_argument("Invalid hex color: " + input);
    };
    auto byte = [&](std::size_t i) -> unsigned char {
        return static_cast<unsigned char>(nibble(color[i]) * 16 + nibble(color[i + 1]));
    };

    return Color(byte(0), byte(2), byte(4), 255);
}

inline std::string color_to_hex(Color c) {
    char buf[8];
    snprintf(buf, sizeof(buf), "#%02X%02X%02X", c.r, c.g, c.b);
    return buf;
}
