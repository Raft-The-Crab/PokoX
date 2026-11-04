/**
 * @file plane.h
 * @brief Plane for collision detection and frustum culling.
 */

#ifndef POKO_MATH_PLANE_H
#define POKO_MATH_PLANE_H

#include "vec3.h"
#include "constants.h"

namespace poko::math {

struct Plane {
    Vec3 normal;
    float distance;

    Plane() : normal(Vec3::up()), distance(0.0f) {}
    Plane(const Vec3& normal, float distance) : normal(normal), distance(distance) {}

    // Create plane from point and normal
    static Plane from_point_normal(const Vec3& point, const Vec3& normal) {
        return Plane(normal.normalized(), -normal.dot(point));
    }

    // Create plane from three points (counter-clockwise winding)
    static Plane from_points(const Vec3& a, const Vec3& b, const Vec3& c) {
        Vec3 ab = b - a;
        Vec3 ac = c - a;
        Vec3 normal = ab.cross(ac).normalized();
        return from_point_normal(a, normal);
    }

    // Distance from point to plane (positive = in front, negative = behind)
    float distance_to_point(const Vec3& point) const {
        return normal.dot(point) + distance;
    }

    // Check if point is on or in front of plane
    bool contains(const Vec3& point) const {
        return distance_to_point(point) >= -EPSILON;
    }

    // Project point onto plane
    Vec3 project_point(const Vec3& point) const {
        float d = distance_to_point(point);
        return point - normal * d;
    }

    // Ray-plane intersection
    // Returns distance along ray, or -1 if no intersection
    float intersect_ray(const Vec3& ray_origin, const Vec3& ray_direction) const {
        float denom = normal.dot(ray_direction);
        if (approximately_zero(denom)) {
            return -1.0f; // Ray is parallel to plane
        }
        float t = -(normal.dot(ray_origin) + distance) / denom;
        return (t >= 0.0f) ? t : -1.0f;
    }
};

} // namespace poko::math

#endif // POKO_MATH_PLANE_H
