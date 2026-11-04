/**
 * @file mat4.h
 * @brief 4x4 matrix math operations for transformations.
 */

#ifndef POKO_MATH_MAT4_H
#define POKO_MATH_MAT4_H

#include "constants.h"
#include "vec3.h"
#include "vec4.h"
#include <cmath>

namespace poko::math {

struct Mat4 {
    // Column-major layout (OpenGL/GLSL standard)
    // Column 0: m[0], m[1], m[2], m[3]
    // Column 1: m[4], m[5], m[6], m[7]
    // Column 2: m[8], m[9], m[10], m[11]
    // Column 3: m[12], m[13], m[14], m[15]
    float m[16] = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };

    Mat4() = default;

    // Identity matrix
    static Mat4 identity() {
        return Mat4();
    }

    // Translation matrix (column 3)
    static Mat4 translation(float x, float y, float z) {
        Mat4 result;
        result.m[12] = x;
        result.m[13] = y;
        result.m[14] = z;
        return result;
    }

    static Mat4 translation(const Vec3& v) {
        return translation(v.x, v.y, v.z);
    }

    // Scale matrix (diagonal)
    static Mat4 scale(float x, float y, float z) {
        Mat4 result;
        result.m[0] = x;
        result.m[5] = y;
        result.m[10] = z;
        return result;
    }

    static Mat4 scale(const Vec3& v) {
        return scale(v.x, v.y, v.z);
    }

    // Rotation around X axis (radians, column-major)
    static Mat4 rotation_x(float angle) {
        Mat4 result;
        float c = std::cos(angle);
        float s = std::sin(angle);
        result.m[5] = c;
        result.m[6] = s;
        result.m[9] = -s;
        result.m[10] = c;
        return result;
    }

    // Rotation around Y axis (radians, column-major)
    static Mat4 rotation_y(float angle) {
        Mat4 result;
        float c = std::cos(angle);
        float s = std::sin(angle);
        result.m[0] = c;
        result.m[2] = -s;
        result.m[8] = s;
        result.m[10] = c;
        return result;
    }

    // Rotation around Z axis (radians, column-major)
    static Mat4 rotation_z(float angle) {
        Mat4 result;
        float c = std::cos(angle);
        float s = std::sin(angle);
        result.m[0] = c;
        result.m[1] = s;
        result.m[4] = -s;
        result.m[5] = c;
        return result;
    }

    // Matrix multiplication (column-major)
    Mat4 operator*(const Mat4& other) const {
        Mat4 result;
        for (int col = 0; col < 4; ++col) {
            for (int row = 0; row < 4; ++row) {
                result.m[col * 4 + row] = 0.0f;
                for (int k = 0; k < 4; ++k) {
                    result.m[col * 4 + row] += m[k * 4 + row] * other.m[col * 4 + k];
                }
            }
        }
        return result;
    }

    // Vector transformation (point, column-major)
    Vec4 transform_point(const Vec3& v) const {
        return Vec4(
            m[0] * v.x + m[4] * v.y + m[8] * v.z + m[12],
            m[1] * v.x + m[5] * v.y + m[9] * v.z + m[13],
            m[2] * v.x + m[6] * v.y + m[10] * v.z + m[14],
            m[3] * v.x + m[7] * v.y + m[11] * v.z + m[15]
        );
    }

    // Vector transformation (direction, no translation, column-major)
    Vec3 transform_direction(const Vec3& v) const {
        return Vec3(
            m[0] * v.x + m[4] * v.y + m[8] * v.z,
            m[1] * v.x + m[5] * v.y + m[9] * v.z,
            m[2] * v.x + m[6] * v.y + m[10] * v.z
        );
    }

    // Perspective projection matrix (column-major, OpenGL convention)
    static Mat4 perspective(float fov_y, float aspect, float near_z, float far_z) {
        Mat4 result;
        float tan_half_fov = std::tan(fov_y * 0.5f);
        result.m[0] = 1.0f / (aspect * tan_half_fov);
        result.m[5] = 1.0f / tan_half_fov;
        result.m[10] = -(far_z + near_z) / (far_z - near_z);
        result.m[11] = -1.0f;
        result.m[14] = -(2.0f * far_z * near_z) / (far_z - near_z);
        result.m[15] = 0.0f;
        return result;
    }

    // Orthographic projection matrix (column-major)
    static Mat4 orthographic(float left, float right, float bottom, float top, float near_z, float far_z) {
        Mat4 result;
        result.m[0] = 2.0f / (right - left);
        result.m[5] = 2.0f / (top - bottom);
        result.m[10] = -2.0f / (far_z - near_z);
        result.m[12] = -(right + left) / (right - left);
        result.m[13] = -(top + bottom) / (top - bottom);
        result.m[14] = -(far_z + near_z) / (far_z - near_z);
        return result;
    }

    // Look-at matrix (column-major, view matrix)
    static Mat4 look_at(const Vec3& eye, const Vec3& target, const Vec3& up) {
        Vec3 f = (target - eye).normalized();
        Vec3 s = f.cross(up).normalized();
        Vec3 u = s.cross(f);

        Mat4 result;
        result.m[0] = s.x;
        result.m[4] = s.y;
        result.m[8] = s.z;
        result.m[1] = u.x;
        result.m[5] = u.y;
        result.m[9] = u.z;
        result.m[2] = -f.x;
        result.m[6] = -f.y;
        result.m[10] = -f.z;
        result.m[12] = -s.dot(eye);
        result.m[13] = -u.dot(eye);
        result.m[14] = f.dot(eye);
        return result;
    }

    // Transpose matrix
    Mat4 transpose() const {
        Mat4 result;
        for (int row = 0; row < 4; ++row) {
            for (int col = 0; col < 4; ++col) {
                result.m[row * 4 + col] = m[col * 4 + row];
            }
        }
        return result;
    }

    // Determinant (4x4)
    float determinant() const {
        // Laplace expansion along first row
        float det = 0.0f;
        for (int i = 0; i < 4; ++i) {
            float sign = (i % 2 == 0) ? 1.0f : -1.0f;
            det += sign * m[i] * minor(0, i);
        }
        return det;
    }

    // Minor (3x3 determinant)
    float minor(int row, int col) const {
        // Extract 3x3 submatrix excluding row and col
        float sub[9];
        int idx = 0;
        for (int r = 0; r < 4; ++r) {
            if (r == row) continue;
            for (int c = 0; c < 4; ++c) {
                if (c == col) continue;
                sub[idx++] = m[c * 4 + r];
            }
        }
        // 3x3 determinant
        return sub[0] * (sub[4] * sub[8] - sub[5] * sub[7])
             - sub[1] * (sub[3] * sub[8] - sub[5] * sub[6])
             + sub[2] * (sub[3] * sub[7] - sub[4] * sub[6]);
    }

    // Inverse matrix
    Mat4 inverse() const {
        Mat4 result;
        float det = determinant();

        if (approximately_zero(det)) {
            // Return identity if singular
            return Mat4::identity();
        }

        float inv_det = 1.0f / det;

        // Compute cofactor matrix and transpose (adjugate)
        for (int row = 0; row < 4; ++row) {
            for (int col = 0; col < 4; ++col) {
                float sign = ((row + col) % 2 == 0) ? 1.0f : -1.0f;
                result.m[col * 4 + row] = sign * minor(row, col) * inv_det;
            }
        }

        return result;
    }

    // Linear interpolation
    static Mat4 lerp(const Mat4& a, const Mat4& b, float t) {
        Mat4 result;
        for (int i = 0; i < 16; ++i) {
            result.m[i] = a.m[i] + (b.m[i] - a.m[i]) * t;
        }
        return result;
    }
};

} // namespace poko::math

#endif // POKO_MATH_MAT4_H
