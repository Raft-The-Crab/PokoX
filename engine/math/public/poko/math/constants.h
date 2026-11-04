/**
 * @file constants.h
 * @brief Math constants and floating-point comparison helpers.
 */

#ifndef POKO_MATH_CONSTANTS_H
#define POKO_MATH_CONSTANTS_H

#include <cmath>

namespace poko::math {

// Epsilon for floating-point comparisons
constexpr float EPSILON = 0.000001f;
constexpr float EPSILON_SQ = EPSILON * EPSILON;

// Mathematical constants
constexpr float PI = 3.14159265358979323846f;
constexpr float TWO_PI = 2.0f * PI;
constexpr float HALF_PI = PI * 0.5f;
constexpr float DEG_TO_RAD = PI / 180.0f;
constexpr float RAD_TO_DEG = 180.0f / PI;

// Floating-point comparison helpers
inline bool approximately_equal(float a, float b, float epsilon = EPSILON) {
    return std::abs(a - b) < epsilon;
}

inline bool approximately_zero(float a, float epsilon = EPSILON) {
    return std::abs(a) < epsilon;
}

inline bool greater_than(float a, float b, float epsilon = EPSILON) {
    return a - b > epsilon;
}

inline bool less_than(float a, float b, float epsilon = EPSILON) {
    return b - a > epsilon;
}

inline bool between(float value, float min, float max) {
    return value >= min && value <= max;
}

} // namespace poko::math

#endif // POKO_MATH_CONSTANTS_H
