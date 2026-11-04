/**
 * @file aabb.h
 * @brief Axis-Aligned Bounding Box for collision detection.
 */

#ifndef POKO_MATH_AABB_H
#define POKO_MATH_AABB_H

#include "vec3.h"
#include "constants.h"

namespace poko::math {

struct AABB {
    Vec3 min;
    Vec3 max;

    AABB() : min(Vec3(0.0f, 0.0f, 0.0f)), max(Vec3(0.0f, 0.0f, 0.0f)) {}
    AABB(const Vec3& min, const Vec3& max) : min(min), max(max) {}

    // Create from center and extents
    static AABB from_center_extents(const Vec3& center, const Vec3& extents) {
        return AABB(center - extents, center + extents);
    }

    // Check if point is inside AABB
    bool contains(const Vec3& point) const {
        return point.x >= min.x && point.x <= max.x &&
               point.y >= min.y && point.y <= max.y &&
               point.z >= min.z && point.z <= max.z;
    }

    // Check if two AABBs intersect
    bool intersects(const AABB& other) const {
        return min.x <= other.max.x && max.x >= other.min.x &&
               min.y <= other.max.y && max.y >= other.min.y &&
               min.z <= other.max.z && max.z >= other.min.z;
    }

    // Get center of AABB
    Vec3 center() const {
        return (min + max) * 0.5f;
    }

    // Get extents (half-size) of AABB
    Vec3 extents() const {
        return (max - min) * 0.5f;
    }

    // Get size of AABB
    Vec3 size() const {
        return max - min;
    }

    // Expand AABB to include point
    void expand(const Vec3& point) {
        if (point.x < min.x) min.x = point.x;
        if (point.y < min.y) min.y = point.y;
        if (point.z < min.z) min.z = point.z;
        if (point.x > max.x) max.x = point.x;
        if (point.y > max.y) max.y = point.y;
        if (point.z > max.z) max.z = point.z;
    }

    // Merge two AABBs
    static AABB merge(const AABB& a, const AABB& b) {
        AABB result;
        result.min.x = (a.min.x < b.min.x) ? a.min.x : b.min.x;
        result.min.y = (a.min.y < b.min.y) ? a.min.y : b.min.y;
        result.min.z = (a.min.z < b.min.z) ? a.min.z : b.min.z;
        result.max.x = (a.max.x > b.max.x) ? a.max.x : b.max.x;
        result.max.y = (a.max.y > b.max.y) ? a.max.y : b.max.y;
        result.max.z = (a.max.z > b.max.z) ? a.max.z : b.max.z;
        return result;
    }
};

} // namespace poko::math

#endif // POKO_MATH_AABB_H
