/**
 * @file vec3.h
 * @brief 3D vector math operations.
 */

#ifndef POKO_MATH_VEC3_H
#define POKO_MATH_VEC3_H

#include "constants.h"
#include <cmath>

namespace poko::math {

struct Vec3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    Vec3() = default;
    constexpr Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

    // Vector operations
    Vec3 operator+(const Vec3& other) const { return Vec3(x + other.x, y + other.y, z + other.z); }
    Vec3 operator-(const Vec3& other) const { return Vec3(x - other.x, y - other.y, z - other.z); }
    Vec3 operator*(float scalar) const { return Vec3(x * scalar, y * scalar, z * scalar); }
    Vec3 operator/(float scalar) const { return Vec3(x / scalar, y / scalar, z / scalar); }

    Vec3& operator+=(const Vec3& other) { x += other.x; y += other.y; z += other.z; return *this; }
    Vec3& operator-=(const Vec3& other) { x -= other.x; y -= other.y; z -= other.z; return *this; }
    Vec3& operator*=(float scalar) { x *= scalar; y *= scalar; z *= scalar; return *this; }
    Vec3& operator/=(float scalar) { x /= scalar; y /= scalar; z /= scalar; return *this; }

    // Dot product
    float dot(const Vec3& other) const { return x * other.x + y * other.y + z * other.z; }

    // Cross product
    Vec3 cross(const Vec3& other) const {
        return Vec3(
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        );
    }

    // Component-wise multiplication
    Vec3 operator*(const Vec3& other) const {
        return Vec3(x * other.x, y * other.y, z * other.z);
    }

    // Length
    float length() const { return std::sqrt(x * x + y * y + z * z); }
    float length_squared() const { return x * x + y * y + z * z; }

    // Normalize
    Vec3 normalized() const {
        float len = length();
        if (len > EPSILON) {
            return Vec3(x / len, y / len, z / len);
        }
        return Vec3(0.0f, 0.0f, 0.0f);
    }

    // Distance
    float distance_to(const Vec3& other) const {
        float dx = x - other.x;
        float dy = y - other.y;
        float dz = z - other.z;
        return std::sqrt(dx * dx + dy * dy + dz * dz);
    }

    // Zero check
    bool is_zero() const { return length_squared() < EPSILON_SQ; }

    // Linear interpolation
    static Vec3 lerp(const Vec3& a, const Vec3& b, float t) {
        return a + (b - a) * t;
    }

    // Static helpers
    static Vec3 zero() { return Vec3(0.0f, 0.0f, 0.0f); }
    static Vec3 one() { return Vec3(1.0f, 1.0f, 1.0f); }
    static Vec3 up() { return Vec3(0.0f, 1.0f, 0.0f); }
    static Vec3 down() { return Vec3(0.0f, -1.0f, 0.0f); }
    static Vec3 left() { return Vec3(-1.0f, 0.0f, 0.0f); }
    static Vec3 right() { return Vec3(1.0f, 0.0f, 0.0f); }
    static Vec3 forward() { return Vec3(0.0f, 0.0f, 1.0f); }
    static Vec3 back() { return Vec3(0.0f, 0.0f, -1.0f); }
};

} // namespace poko::math

#endif // POKO_MATH_VEC3_H
