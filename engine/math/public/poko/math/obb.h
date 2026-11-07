/**
 * @file obb.h
 * @brief Oriented Bounding Box (OBB) for collision detection.
 */

#ifndef POKO_MATH_OBB_H
#define POKO_MATH_OBB_H

#include "vec3.h"
#include "mat4.h"
#include "constants.h"

namespace poko::math {

struct OBB {
    Vec3 center;
    Vec3 extents;  // Half-sizes along local axes
    Vec3 axes[3];  // Local coordinate axes (normalized)

    OBB() : center(Vec3::zero()), extents(Vec3::one()) {
        axes[0] = Vec3::right();
        axes[1] = Vec3::up();
        axes[2] = Vec3::forward();
    }

    OBB(const Vec3& center, const Vec3& extents, const Mat4& rotation) {
        this->center = center;
        this->extents = extents;
        // Extract rotation matrix columns as axes
        axes[0] = Vec3(rotation.m[0], rotation.m[1], rotation.m[2]).normalized();
        axes[1] = Vec3(rotation.m[4], rotation.m[5], rotation.m[6]).normalized();
        axes[2] = Vec3(rotation.m[8], rotation.m[9], rotation.m[10]).normalized();
    }

    // Check if point is inside OBB
    bool contains(const Vec3& point) const {
        Vec3 local = point - center;
        Vec3 local_axes;
        local_axes.x = local.dot(axes[0]);
        local_axes.y = local.dot(axes[1]);
        local_axes.z = local.dot(axes[2]);

        return std::abs(local_axes.x) <= extents.x &&
               std::abs(local_axes.y) <= extents.y &&
               std::abs(local_axes.z) <= extents.z;
    }

    // Project point onto OBB surface
    Vec3 project_point(const Vec3& point) const {
        Vec3 local = point - center;
        Vec3 local_axes;
        local_axes.x = local.dot(axes[0]);
        local_axes.y = local.dot(axes[1]);
        local_axes.z = local.dot(axes[2]);

        // Clamp to OBB bounds
        local_axes.x = (local_axes.x < -extents.x) ? -extents.x : (local_axes.x > extents.x) ? extents.x : local_axes.x;
        local_axes.y = (local_axes.y < -extents.y) ? -extents.y : (local_axes.y > extents.y) ? extents.y : local_axes.y;
        local_axes.z = (local_axes.z < -extents.z) ? -extents.z : (local_axes.z > extents.z) ? extents.z : local_axes.z;

        // Convert back to world space
        return center + axes[0] * local_axes.x + axes[1] * local_axes.y + axes[2] * local_axes.z;
    }

    // Get world matrix from OBB
    Mat4 to_matrix() const {
        Mat4 result;
        // Rotation from axes
        result.m[0] = axes[0].x; result.m[1] = axes[0].y; result.m[2] = axes[0].z;
        result.m[4] = axes[1].x; result.m[5] = axes[1].y; result.m[6] = axes[1].z;
        result.m[8] = axes[2].x; result.m[9] = axes[2].y; result.m[10] = axes[2].z;
        // Translation
        result.m[12] = center.x;
        result.m[13] = center.y;
        result.m[14] = center.z;
        // Scale from extents
        result.m[0] *= extents.x;
        result.m[5] *= extents.y;
        result.m[10] *= extents.z;
        return result;
    }
};

} // namespace poko::math

#endif // POKO_MATH_OBB_H
