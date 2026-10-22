#pragma once

#include <concepts>
#include <type_traits>

struct Vec2 {
    float x = 0.0f, y = 0.0f;

    constexpr Vec2() = default;
    constexpr Vec2(float x, float y) : x(x), y(y) {}
    constexpr bool operator==(const Vec2&) const = default;

    template <class T>
        requires(!std::same_as<T, Vec2> && std::is_aggregate_v<T> && sizeof(T) == sizeof(float) * 2 &&
                 requires(const T& t) { t.x; t.y; })
    constexpr Vec2(const T& v) : x(v.x), y(v.y) {}

    template <class T>
        requires(!std::same_as<T, Vec2> && std::is_aggregate_v<T> && sizeof(T) == sizeof(float) * 2 &&
                 requires(T t) { t.x; t.y; })
    constexpr operator T() const {
        return T{x, y};
    }
};

struct Vec3 {
    float x = 0.0f, y = 0.0f, z = 0.0f;

    constexpr Vec3() = default;
    constexpr Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
    constexpr bool operator==(const Vec3&) const = default;

    template <class T>
        requires(!std::same_as<T, Vec3> && std::is_aggregate_v<T> && sizeof(T) == sizeof(float) * 3 &&
                 requires(const T& t) { t.x; t.y; t.z; })
    constexpr Vec3(const T& v) : x(v.x), y(v.y), z(v.z) {}

    template <class T>
        requires(!std::same_as<T, Vec3> && std::is_aggregate_v<T> && sizeof(T) == sizeof(float) * 3 &&
                 requires(T t) { t.x; t.y; t.z; })
    constexpr operator T() const {
        return T{x, y, z};
    }
};

struct Rect {
    float x = 0.0f, y = 0.0f, width = 0.0f, height = 0.0f;

    constexpr Rect() = default;
    constexpr Rect(float x, float y, float width, float height) : x(x), y(y), width(width), height(height) {}
    constexpr bool operator==(const Rect&) const = default;

    template <class T>
        requires(!std::same_as<T, Rect> && std::is_aggregate_v<T> && sizeof(T) == sizeof(float) * 4 &&
                 requires(const T& t) { t.x; t.y; t.width; t.height; })
    constexpr Rect(const T& r) : x(r.x), y(r.y), width(r.width), height(r.height) {}

    template <class T>
        requires(!std::same_as<T, Rect> && std::is_aggregate_v<T> && sizeof(T) == sizeof(float) * 4 &&
                 requires(T t) { t.x; t.y; t.width; t.height; })
    constexpr operator T() const {
        return T{x, y, width, height};
    }
};

// 4x4 matrix, column-major: mC R is column C, row R (raylib's field order).
struct Mat4 {
    float m0 = 0, m4 = 0, m8 = 0, m12 = 0;
    float m1 = 0, m5 = 0, m9 = 0, m13 = 0;
    float m2 = 0, m6 = 0, m10 = 0, m14 = 0;
    float m3 = 0, m7 = 0, m11 = 0, m15 = 0;

    constexpr Mat4() = default;

    template <class T>
        requires(!std::same_as<T, Mat4> && std::is_aggregate_v<T> && sizeof(T) == sizeof(float) * 16 &&
                 requires(const T& t) { t.m0; t.m15; })
    constexpr Mat4(const T& m)
        : m0(m.m0), m4(m.m4), m8(m.m8), m12(m.m12), m1(m.m1), m5(m.m5), m9(m.m9), m13(m.m13), m2(m.m2), m6(m.m6),
          m10(m.m10), m14(m.m14), m3(m.m3), m7(m.m7), m11(m.m11), m15(m.m15) {}

    template <class T>
        requires(!std::same_as<T, Mat4> && std::is_aggregate_v<T> && sizeof(T) == sizeof(float) * 16 &&
                 requires(T t) { t.m0; t.m15; })
    constexpr operator T() const {
        return T{m0, m4, m8, m12, m1, m5, m9, m13, m2, m6, m10, m14, m3, m7, m11, m15};
    }
};
