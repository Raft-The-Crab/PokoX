/**
 * @file ray.h
 * @brief Ray for raycasting and intersection tests.
 */

#ifndef POKO_MATH_RAY_H
#define POKO_MATH_RAY_H

#include "vec3.h"
#include "aabb.h"
#include "plane.h"
#include "constants.h"
#include <algorithm>
#include <limits>

namespace poko::math {

struct Ray {
    Vec3 origin;
    Vec3 direction;

    Ray() : origin(Vec3::zero()), direction(Vec3::forward()) {}
    Ray(const Vec3& origin, const Vec3& direction) : origin(origin), direction(direction.normalized()) {}

    // Get point at distance t along ray
    Vec3 point_at(float t) const {
        return origin + direction * t;
    }

    // AABB intersection (slab method)
    // Returns distance to intersection, or -1 if no intersection
    float intersect_aabb(const AABB& aabb) const {
        float t_min = 0.0f;
        float t_max = std::numeric_limits<float>::max();

        float d_vals[3] = {direction.x, direction.y, direction.z};
        float o_vals[3] = {origin.x, origin.y, origin.z};
        float min_vals[3] = {aabb.min.x, aabb.min.y, aabb.min.z};
        float max_vals[3] = {aabb.max.x, aabb.max.y, aabb.max.z};

        for (int i = 0; i < 3; ++i) {
            float inv_d = 1.0f / d_vals[i];
            float t0 = (min_vals[i] - o_vals[i]) * inv_d;
            float t1 = (max_vals[i] - o_vals[i]) * inv_d;

            if (inv_d < 0.0f) {
                std::swap(t0, t1);
            }

            t_min = (t0 > t_min) ? t0 : t_min;
            t_max = (t1 < t_max) ? t1 : t_max;

            if (t_max <= t_min) {
                return -1.0f;
            }
        }

        return t_min;
    }

    // Plane intersection
    float intersect_plane(const Plane& plane) const {
        return plane.intersect_ray(origin, direction);
    }

    // Triangle intersection (Möller-Trumbore algorithm)
    // Returns distance to intersection, or -1 if no intersection
    float intersect_triangle(const Vec3& v0, const Vec3& v1, const Vec3& v2) const {
        Vec3 edge1 = v1 - v0;
        Vec3 edge2 = v2 - v0;
        Vec3 h = direction.cross(edge2);
        float a = edge1.dot(h);

        if (approximately_zero(a)) {
            return -1.0f; // Ray is parallel to triangle
        }

        float f = 1.0f / a;
        Vec3 s = origin - v0;
        float u = f * s.dot(h);

        if (u < 0.0f || u > 1.0f) {
            return -1.0f;
        }

        Vec3 q = s.cross(edge1);
        float v = f * direction.dot(q);

        if (v < 0.0f || u + v > 1.0f) {
            return -1.0f;
        }

        float t = f * edge2.dot(q);

        return (t > EPSILON) ? t : -1.0f;
    }
};

} // namespace poko::math

#endif // POKO_MATH_RAY_H
