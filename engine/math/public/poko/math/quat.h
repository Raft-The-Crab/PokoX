/**
 * @file quat.h
 * @brief Quaternion math operations for rotations.
 */

#ifndef POKO_MATH_QUAT_H
#define POKO_MATH_QUAT_H

#include "constants.h"
#include "vec3.h"
#include "mat4.h"
#include <cmath>

namespace poko::math {

struct Quat {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 1.0f;

    Quat() = default;
    constexpr Quat(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

    // Identity quaternion
    static Quat identity() { return Quat(0.0f, 0.0f, 0.0f, 1.0f); }

    // Quaternion multiplication
    Quat operator*(const Quat& other) const {
        return Quat(
            w * other.x + x * other.w + y * other.z - z * other.y,
            w * other.y - x * other.z + y * other.w + z * other.x,
            w * other.z + x * other.y - y * other.x + z * other.w,
            w * other.w - x * other.x - y * other.y - z * other.z
        );
    }

    // Quaternion addition
    Quat operator+(const Quat& other) const {
        return Quat(x + other.x, y + other.y, z + other.z, w + other.w);
    }

    // Scalar multiplication
    Quat operator*(float scalar) const {
        return Quat(x * scalar, y * scalar, z * scalar, w * scalar);
    }

    // Conjugate
    Quat conjugate() const {
        return Quat(-x, -y, -z, w);
    }

    // Length
    float length() const {
        return std::sqrt(x * x + y * y + z * z + w * w);
    }

    // Normalize
    Quat normalized() const {
        float len = length();
        if (len > EPSILON) {
            return Quat(x / len, y / len, z / len, w / len);
        }
        return Quat(0.0f, 0.0f, 0.0f, 1.0f);
    }

    // From axis-angle (radians)
    static Quat from_axis_angle(const Vec3& axis, float angle) {
        Vec3 a = axis.normalized();
        float half_angle = angle * 0.5f;
        float s = std::sin(half_angle);
        return Quat(a.x * s, a.y * s, a.z * s, std::cos(half_angle));
    }

    // From Euler angles (radians, YXZ order)
    static Quat from_euler(float pitch, float yaw, float roll) {
        float cy = std::cos(yaw * 0.5f);
        float sy = std::sin(yaw * 0.5f);
        float cp = std::cos(pitch * 0.5f);
        float sp = std::sin(pitch * 0.5f);
        float cr = std::cos(roll * 0.5f);
        float sr = std::sin(roll * 0.5f);

        Quat q;
        q.w = cr * cp * cy + sr * sp * sy;
        q.x = sr * cp * cy - cr * sp * sy;
        q.y = cr * sp * cy + sr * cp * sy;
        q.z = cr * cp * sy - sr * sp * cy;
        return q;
    }

    // Rotate vector
    Vec3 rotate(const Vec3& v) const {
        Vec3 qv(x, y, z);
        Vec3 uv = qv.cross(v);
        Vec3 uuv = qv.cross(uv);
        return v + (uv * 2.0f * w) + (uuv * 2.0f);
    }

    // Dot product
    float dot(const Quat& other) const {
        return x * other.x + y * other.y + z * other.z + w * other.w;
    }

    // Linear interpolation
    static Quat lerp(const Quat& a, const Quat& b, float t) {
        return (a * (1.0f - t) + b * t).normalized();
    }

    // Spherical linear interpolation
    static Quat slerp(const Quat& a, const Quat& b, float t) {
        float dot = a.dot(b);
        Quat temp;

        if (dot < 0.0f) {
            temp = b * -1.0f;
            dot = -dot;
        } else {
            temp = b;
        }

        if (dot > 0.9995f) {
            return lerp(a, temp, t);
        }

        float theta0 = std::acos(dot);
        float theta = theta0 * t;
        float sin_theta = std::sin(theta);
        float sin_theta0 = std::sin(theta0);

        float s0 = std::cos(theta) - dot * sin_theta / sin_theta0;
        float s1 = sin_theta / sin_theta0;

        return a * s0 + temp * s1;
    }

    // Convert to rotation matrix (column-major)
    Mat4 to_matrix() const {
        Mat4 result;
        float xx = x * x;
        float yy = y * y;
        float zz = z * z;
        float xy = x * y;
        float xz = x * z;
        float yz = y * z;
        float wx = w * x;
        float wy = w * y;
        float wz = w * z;

        // Column 0
        result.m[0] = 1.0f - 2.0f * (yy + zz);
        result.m[1] = 2.0f * (xy + wz);
        result.m[2] = 2.0f * (xz - wy);
        result.m[3] = 0.0f;

        // Column 1
        result.m[4] = 2.0f * (xy - wz);
        result.m[5] = 1.0f - 2.0f * (xx + zz);
        result.m[6] = 2.0f * (yz + wx);
        result.m[7] = 0.0f;

        // Column 2
        result.m[8] = 2.0f * (xz + wy);
        result.m[9] = 2.0f * (yz - wx);
        result.m[10] = 1.0f - 2.0f * (xx + yy);
        result.m[11] = 0.0f;

        // Column 3 (identity for rotation)
        result.m[12] = 0.0f;
        result.m[13] = 0.0f;
        result.m[14] = 0.0f;
        result.m[15] = 1.0f;

        return result;
    }
};

} // namespace poko::math

#endif // POKO_MATH_QUAT_H
