#pragma once

#include <concepts>
#include <type_traits>

// Backend-neutral GPU texture handle. Layout matches raylib's Texture, so it
// converts implicitly to/from it (same pattern as Vec2 in math_types.h).
struct Texture {
    unsigned int id = 0;
    int width = 0, height = 0, mipmaps = 0, format = 0;

    constexpr Texture() = default;
    constexpr bool operator==(const Texture&) const = default;

    template<class T>
        requires(!std::same_as<T, Texture> && std::is_aggregate_v<T> &&
                 sizeof(T) == sizeof(unsigned int) + 4 * sizeof(int) &&
                 requires(const T& t) {
                     t.id;
                     t.width;
                     t.height;
                     t.mipmaps;
                     t.format;
                 })
    constexpr Texture(const T& t) : id(t.id), width(t.width), height(t.height), mipmaps(t.mipmaps), format(t.format) {}

    template<class T>
        requires(!std::same_as<T, Texture> && std::is_aggregate_v<T> &&
                 sizeof(T) == sizeof(unsigned int) + 4 * sizeof(int) &&
                 requires(T t) {
                     t.id;
                     t.width;
                     t.height;
                     t.mipmaps;
                     t.format;
                 })
    constexpr operator T() const {
        return T{id, width, height, mipmaps, format};
    }
};
