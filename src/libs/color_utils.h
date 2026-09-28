#pragma once

#include <cstdio>
#include <stdexcept>
#include <string>
#include "ray.h" // IWYU pragma: keep

inline ray::Color parse_hex_color(const std::string& input) {
    std::string color = input;
    if (!color.empty() && color[0] == '#')
        color = color.substr(1);
    if (color.size() == 3)
        color = std::string{color[0], color[0], color[1], color[1], color[2], color[2]};
    if (color.size() != 6)
        throw std::invalid_argument("Invalid hex color: " + input);

    auto nibble = [&](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        throw std::invalid_argument("Invalid hex color: " + input);
    };
    auto byte = [&](std::size_t i) -> unsigned char {
        return static_cast<unsigned char>(nibble(color[i]) * 16 + nibble(color[i + 1]));
    };

    return ray::Color(byte(0), byte(2), byte(4), 255);
}

inline std::string color_to_hex(ray::Color c) {
    char buf[8];
    snprintf(buf, sizeof(buf), "#%02X%02X%02X", c.r, c.g, c.b);
    return buf;
}
