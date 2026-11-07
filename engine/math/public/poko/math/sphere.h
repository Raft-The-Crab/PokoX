/**
 * @file sphere.h
 * @brief Sphere for collision detection and culling.
 */

#ifndef POKO_MATH_SPHERE_H
#define POKO_MATH_SPHERE_H

#include "vec3.h"
#include "constants.h"

namespace poko::math {

struct Sphere {
    Vec3 center;
    float radius;

    Sphere() : center(Vec3::zero()), radius(0.0f) {}
    Sphere(const Vec3& center, float radius) : center(center), radius(radius) {}

    // Check if point is inside sphere
    bool contains(const Vec3& point) const {
        return (point - center).length_squared() <= (radius * radius);
    }

    // Check if two spheres intersect
    bool intersects(const Sphere& other) const {
        float dist_sq = (center - other.center).length_squared();
        float radius_sum = radius + other.radius;
        return dist_sq <= (radius_sum * radius_sum);
    }

    // Get closest point on sphere surface to point
    Vec3 closest_point(const Vec3& point) const {
        Vec3 dir = (point - center).normalized();
        return center + dir * radius;
    }

    // Check if sphere intersects AABB
    bool intersects_aabb(const AABB& aabb) const {
        // Find closest point on AABB to sphere center
        Vec3 closest;
        closest.x = (center.x < aabb.min.x) ? aabb.min.x : (center.x > aabb.max.x) ? aabb.max.x : center.x;
        closest.y = (center.y < aabb.min.y) ? aabb.min.y : (center.y > aabb.max.y) ? aabb.max.y : center.y;
        closest.z = (center.z < aabb.min.z) ? aabb.min.z : (center.z > aabb.max.z) ? aabb.max.z : center.z;

        float dist_sq = (center - closest).length_squared();
        return dist_sq <= (radius * radius);
    }

    // Merge two spheres
    static Sphere merge(const Sphere& a, const Sphere& b) {
        Vec3 dir = (b.center - a.center);
        float dist = dir.length();
        if (approximately_zero(dist)) {
            return Sphere(a.center, (a.radius > b.radius) ? a.radius : b.radius);
        }
        dir = dir.normalized();
        float new_radius = (dist + a.radius + b.radius) * 0.5f;
        Vec3 new_center = a.center + dir * ((dist + b.radius - a.radius) * 0.5f);
        return Sphere(new_center, new_radius);
    }
};

} // namespace poko::math

#endif // POKO_MATH_SPHERE_H
