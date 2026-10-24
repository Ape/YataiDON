#pragma once

#include <bit>
#include <concepts>
#include <type_traits>

#include "math_types.h"

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

// Same-layout bit conversion to/from a backend struct (evaluated lazily, so the
// class is complete when sizeof is checked).
template <class Self, class T>
concept bit_convertible = !std::same_as<Self, T> && std::is_trivially_copyable_v<T> && sizeof(T) == sizeof(Self);

struct Camera2D {
    Vec2 offset;
    Vec2 target;
    float rotation = 0.0f;
    float zoom = 1.0f;

    constexpr Camera2D() = default;
    constexpr Camera2D(Vec2 offset, Vec2 target, float rotation, float zoom)
        : offset(offset), target(target), rotation(rotation), zoom(zoom) {}
    template <class T>
        requires bit_convertible<Camera2D, T>
    constexpr Camera2D(const T& t) {
        *this = std::bit_cast<Camera2D>(t);
    }
    template <class T>
        requires bit_convertible<Camera2D, T>
    constexpr operator T() const {
        return std::bit_cast<T>(*this);
    }
};

namespace camera_projection {
inline constexpr int PERSPECTIVE = 0;
inline constexpr int ORTHOGRAPHIC = 1;
} // namespace camera_projection

struct Camera3D {
    Vec3 position;
    Vec3 target;
    Vec3 up;
    float fovy = 0.0f;
    int projection = camera_projection::PERSPECTIVE;

    constexpr Camera3D() = default;
    constexpr Camera3D(Vec3 position, Vec3 target, Vec3 up, float fovy, int projection)
        : position(position), target(target), up(up), fovy(fovy), projection(projection) {}
    template <class T>
        requires bit_convertible<Camera3D, T>
    constexpr Camera3D(const T& t) {
        *this = std::bit_cast<Camera3D>(t);
    }
    template <class T>
        requires bit_convertible<Camera3D, T>
    constexpr operator T() const {
        return std::bit_cast<T>(*this);
    }
};

struct Shader {
    unsigned int id = 0;
    int* locs = nullptr;

    constexpr Shader() = default;
    template <class T>
        requires bit_convertible<Shader, T>
    constexpr Shader(const T& t) {
        *this = std::bit_cast<Shader>(t);
    }
    template <class T>
        requires bit_convertible<Shader, T>
    constexpr operator T() const {
        return std::bit_cast<T>(*this);
    }
};

struct RenderTarget {
    unsigned int id = 0;
    Texture texture;
    Texture depth;

    constexpr RenderTarget() = default;
    template <class T>
        requires bit_convertible<RenderTarget, T>
    constexpr RenderTarget(const T& t) {
        *this = std::bit_cast<RenderTarget>(t);
    }
    template <class T>
        requires bit_convertible<RenderTarget, T>
    constexpr operator T() const {
        return std::bit_cast<T>(*this);
    }
};
