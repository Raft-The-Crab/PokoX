/**
 * @file constants.h
 * @brief Math constants and floating-point comparison helpers.
 */

#ifndef POKO_MATH_CONSTANTS_H
#define POKO_MATH_CONSTANTS_H

#include <cmath>
#include <limits>
#include <cstdint>

namespace poko::math {

// Epsilon for floating-point comparisons
constexpr float EPSILON = 0.000001f;
constexpr float EPSILON_SQ = EPSILON * EPSILON;
constexpr float EPSILON_HALF = EPSILON * 0.5f;

// Mathematical constants
constexpr float PI = 3.14159265358979323846f;
constexpr float TWO_PI = 2.0f * PI;
constexpr float HALF_PI = PI * 0.5f;
constexpr float QUARTER_PI = PI * 0.25f;
constexpr float DEG_TO_RAD = PI / 180.0f;
constexpr float RAD_TO_DEG = 180.0f / PI;

// Float limits
constexpr float FLOAT_MAX = std::numeric_limits<float>::max();
constexpr float FLOAT_MIN = std::numeric_limits<float>::lowest();
constexpr float FLOAT_INFINITY = std::numeric_limits<float>::infinity();

// Floating-point comparison helpers
inline bool approximately_equal(float a, float b, float epsilon = EPSILON) {
    return std::abs(a - b) < epsilon;
}

inline bool approximately_equal_relaxed(float a, float b, float epsilon = EPSILON_HALF) {
    return std::abs(a - b) < epsilon;
}

inline bool approximately_zero(float a, float epsilon = EPSILON) {
    return std::abs(a) < epsilon;
}

inline bool approximately_zero_relaxed(float a, float epsilon = EPSILON_HALF) {
    return std::abs(a) < epsilon;
}

inline bool greater_than(float a, float b, float epsilon = EPSILON) {
    return a - b > epsilon;
}

inline bool greater_than_or_equal(float a, float b, float epsilon = EPSILON) {
    return a - b >= -epsilon;
}

inline bool less_than(float a, float b, float epsilon = EPSILON) {
    return b - a > epsilon;
}

inline bool less_than_or_equal(float a, float b, float epsilon = EPSILON) {
    return b - a >= -epsilon;
}

inline bool between(float value, float min, float max) {
    return value >= min - EPSILON && value <= max + EPSILON;
}

inline bool between_relaxed(float value, float min, float max) {
    return value >= min - EPSILON_HALF && value <= max + EPSILON_HALF;
}

// Clamp helper
inline float clamp(float value, float min, float max) {
    return value < min ? min : (value > max ? max : value);
}

// Lerp helper
inline float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

// Inverse lerp helper
inline float inverse_lerp(float a, float b, float value) {
    return approximately_zero(b - a) ? 0.0f : (value - a) / (b - a);
}

// Smooth step helper
inline float smooth_step(float edge0, float edge1, float x) {
    float t = clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

// Smoothstep with edges
inline float smoothstep(float edge0, float edge1, float x) {
    return smooth_step(edge0, edge1, x);
}

// Remap helper
inline float remap(float value, float in_min, float in_max, float out_min, float out_max) {
    return out_min + (value - in_min) * (out_max - out_min) / (in_max - in_min);
}

// Move towards helper
inline float move_towards(float current, float target, float max_delta) {
    float delta = target - current;
    if (std::abs(delta) <= max_delta) {
        return target;
    }
    return current + (delta > 0.0f ? max_delta : -max_delta);
}

// Angle helpers (radians)
inline float normalize_angle(float angle) {
    // Normalize angle to [-PI, PI]
    while (angle > PI) angle -= TWO_PI;
    while (angle < -PI) angle += TWO_PI;
    return angle;
}

inline float angle_lerp(float a, float b, float t) {
    float diff = normalize_angle(b - a);
    return a + diff * t;
}

inline float angle_difference(float a, float b) {
    return normalize_angle(b - a);
}

// Vector helpers
inline float dot3(const float* a, const float* b) {
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

inline void cross3(const float* a, const float* b, float* result) {
    result[0] = a[1] * b[2] - a[2] * b[1];
    result[1] = a[2] * b[0] - a[0] * b[2];
    result[2] = a[0] * b[1] - a[1] * b[0];
}

// Matrix helpers
inline bool is_identity_matrix(const float* matrix, int size = 16) {
    for (int i = 0; i < size; ++i) {
        float expected = (i % 5 == 0) ? 1.0f : 0.0f;  // Diagonal elements
        if (std::abs(matrix[i] - expected) > EPSILON) {
            return false;
        }
    }
    return true;
}

// Random number helpers (simple LCG for game use)
inline uint32_t random_seed(uint32_t seed) {
    static uint32_t state = 0;
    if (seed != 0) {
        state = seed;
    }
    state = state * 1664525u + 1013904223u;
    return state;
}

inline float random_float() {
    return static_cast<float>(random_seed(0)) / static_cast<float>(UINT32_MAX);
}

inline float random_range(float min, float max) {
    return min + random_float() * (max - min);
}

} // namespace poko::math

#endif // POKO_MATH_CONSTANTS_H
