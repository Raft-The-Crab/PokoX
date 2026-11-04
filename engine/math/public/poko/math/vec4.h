/**
 * @file vec4.h
 * @brief 4D vector math operations.
 */

#ifndef POKO_MATH_VEC4_H
#define POKO_MATH_VEC4_H

#include "constants.h"
#include <cmath>

namespace poko::math {

struct Vec4 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 0.0f;

    Vec4() = default;
    constexpr Vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

    // Vector operations
    Vec4 operator+(const Vec4& other) const { return Vec4(x + other.x, y + other.y, z + other.z, w + other.w); }
    Vec4 operator-(const Vec4& other) const { return Vec4(x - other.x, y - other.y, z - other.z, w - other.w); }
    Vec4 operator*(float scalar) const { return Vec4(x * scalar, y * scalar, z * scalar, w * scalar); }
    Vec4 operator/(float scalar) const { return Vec4(x / scalar, y / scalar, z / scalar, w / scalar); }

    Vec4& operator+=(const Vec4& other) { x += other.x; y += other.y; z += other.z; w += other.w; return *this; }
    Vec4& operator-=(const Vec4& other) { x -= other.x; y -= other.y; z -= other.z; w -= other.w; return *this; }
    Vec4& operator*=(float scalar) { x *= scalar; y *= scalar; z *= scalar; w *= scalar; return *this; }
    Vec4& operator/=(float scalar) { x /= scalar; y /= scalar; z /= scalar; w /= scalar; return *this; }

    // Dot product
    float dot(const Vec4& other) const { return x * other.x + y * other.y + z * other.z + w * other.w; }

    // Length
    float length() const { return std::sqrt(x * x + y * y + z * z + w * w); }
    float length_squared() const { return x * x + y * y + z * z + w * w; }

    // Normalize
    Vec4 normalized() const {
        float len = length();
        if (len > EPSILON) {
            return Vec4(x / len, y / len, z / len, w / len);
        }
        return Vec4(0.0f, 0.0f, 0.0f, 0.0f);
    }

    // Zero check
    bool is_zero() const { return length_squared() < EPSILON_SQ; }

    // Linear interpolation
    static Vec4 lerp(const Vec4& a, const Vec4& b, float t) {
        return a + (b - a) * t;
    }

    // Static helpers
    static Vec4 zero() { return Vec4(0.0f, 0.0f, 0.0f, 0.0f); }
    static Vec4 one() { return Vec4(1.0f, 1.0f, 1.0f, 1.0f); }
};

} // namespace poko::math

#endif // POKO_MATH_VEC4_H
