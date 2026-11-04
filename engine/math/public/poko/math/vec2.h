/**
 * @file vec2.h
 * @brief 2D vector math operations.
 */

#ifndef POKO_MATH_VEC2_H
#define POKO_MATH_VEC2_H

#include "constants.h"
#include <cmath>

namespace poko::math {

struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;

    Vec2() = default;
    constexpr Vec2(float x, float y) : x(x), y(y) {}

    // Vector operations
    Vec2 operator+(const Vec2& other) const { return Vec2(x + other.x, y + other.y); }
    Vec2 operator-(const Vec2& other) const { return Vec2(x - other.x, y - other.y); }
    Vec2 operator*(float scalar) const { return Vec2(x * scalar, y * scalar); }
    Vec2 operator/(float scalar) const { return Vec2(x / scalar, y / scalar); }

    Vec2& operator+=(const Vec2& other) { x += other.x; y += other.y; return *this; }
    Vec2& operator-=(const Vec2& other) { x -= other.x; y -= other.y; return *this; }
    Vec2& operator*=(float scalar) { x *= scalar; y *= scalar; return *this; }
    Vec2& operator/=(float scalar) { x /= scalar; y /= scalar; return *this; }

    // Dot product
    float dot(const Vec2& other) const { return x * other.x + y * other.y; }

    // Length
    float length() const { return std::sqrt(x * x + y * y); }
    float length_squared() const { return x * x + y * y; }

    // Normalize
    Vec2 normalized() const {
        float len = length();
        if (len > EPSILON) {
            return Vec2(x / len, y / len);
        }
        return Vec2(0.0f, 0.0f);
    }

    // Distance
    float distance_to(const Vec2& other) const {
        float dx = x - other.x;
        float dy = y - other.y;
        return std::sqrt(dx * dx + dy * dy);
    }

    // Zero check
    bool is_zero() const { return length_squared() < EPSILON_SQ; }

    // Linear interpolation
    static Vec2 lerp(const Vec2& a, const Vec2& b, float t) {
        return a + (b - a) * t;
    }

    // Static helpers
    static Vec2 zero() { return Vec2(0.0f, 0.0f); }
    static Vec2 one() { return Vec2(1.0f, 1.0f); }
    static Vec2 up() { return Vec2(0.0f, 1.0f); }
    static Vec2 right() { return Vec2(1.0f, 0.0f); }
};

} // namespace poko::math

#endif // POKO_MATH_VEC2_H
