/**
 * @file capsule.h
 * @brief Capsule for collision detection.
 */

#ifndef POKO_MATH_CAPSULE_H
#define POKO_MATH_CAPSULE_H

#include "vec3.h"
#include "constants.h"

namespace poko::math {

struct Capsule {
    Vec3 start;
    Vec3 end;
    float radius;

    Capsule() : start(Vec3::zero()), end(Vec3::up()), radius(0.0f) {}
    Capsule(const Vec3& start, const Vec3& end, float radius)
        : start(start), end(end), radius(radius) {}

    // Get capsule center
    Vec3 center() const {
        return (start + end) * 0.5f;
    }

    // Get capsule height (length + 2 * radius)
    float height() const {
        return (end - start).length() + 2.0f * radius;
    }

    // Get capsule axis direction
    Vec3 axis() const {
        return (end - start).normalized();
    }

    // Check if point is inside capsule
    bool contains(const Vec3& point) const {
        Vec3 line_vec = end - start;
        Vec3 point_vec = point - start;
        float line_len_sq = line_vec.length_squared();

        if (approximately_zero(line_len_sq)) {
            // Degenerate to sphere
            return point_vec.length_squared() <= (radius * radius);
        }

        // Project point onto line segment
        float t = point_vec.dot(line_vec) / line_len_sq;
        t = (t < 0.0f) ? 0.0f : (t > 1.0f) ? 1.0f : t;
        Vec3 closest = start + line_vec * t;

        return (point - closest).length_squared() <= (radius * radius);
    }

    // Check if capsule intersects sphere
    bool intersects_sphere(const Sphere& sphere) const {
        // Simplified: check if distance from line segment to sphere center < radius_sum
        Vec3 line_vec = end - start;
        Vec3 sphere_vec = sphere.center - start;
        float line_len_sq = line_vec.length_squared();

        if (approximately_zero(line_len_sq)) {
            // Degenerate to sphere-sphere
            return (sphere.center - start).length_squared() <= ((radius + sphere.radius) * (radius + sphere.radius));
        }

        float t = sphere_vec.dot(line_vec) / line_len_sq;
        t = (t < 0.0f) ? 0.0f : (t > 1.0f) ? 1.0f : t;
        Vec3 closest = start + line_vec * t;

        float dist_sq = (sphere.center - closest).length_squared();
        float radius_sum = radius + sphere.radius;
        return dist_sq <= (radius_sum * radius_sum);
    }
};

} // namespace poko::math

#endif // POKO_MATH_CAPSULE_H
