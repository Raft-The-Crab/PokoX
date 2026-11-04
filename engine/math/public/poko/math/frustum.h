/**
 * @file frustum.h
 * @brief Frustum for culling and view volume operations.
 */

#ifndef POKO_MATH_FRUSTUM_H
#define POKO_MATH_FRUSTUM_H

#include "plane.h"
#include "mat4.h"
#include "aabb.h"

namespace poko::math {

struct Frustum {
    Plane planes[6]; // Left, Right, Top, Bottom, Near, Far

    enum PlaneIndex {
        Left = 0,
        Right = 1,
        Top = 2,
        Bottom = 3,
        Near = 4,
        Far = 5
    };

    Frustum() = default;

    // Extract frustum from view-projection matrix
    static Frustum from_view_projection(const Mat4& view_proj) {
        Frustum frustum;

        // Extract left plane
        frustum.planes[Left].normal = Vec3(
            view_proj.m[3] + view_proj.m[0],
            view_proj.m[7] + view_proj.m[4],
            view_proj.m[11] + view_proj.m[8]
        ).normalized();
        frustum.planes[Left].distance = view_proj.m[15] + view_proj.m[12];

        // Extract right plane
        frustum.planes[Right].normal = Vec3(
            view_proj.m[3] - view_proj.m[0],
            view_proj.m[7] - view_proj.m[4],
            view_proj.m[11] - view_proj.m[8]
        ).normalized();
        frustum.planes[Right].distance = view_proj.m[15] - view_proj.m[12];

        // Extract top plane
        frustum.planes[Top].normal = Vec3(
            view_proj.m[3] - view_proj.m[1],
            view_proj.m[7] - view_proj.m[5],
            view_proj.m[11] - view_proj.m[9]
        ).normalized();
        frustum.planes[Top].distance = view_proj.m[15] - view_proj.m[13];

        // Extract bottom plane
        frustum.planes[Bottom].normal = Vec3(
            view_proj.m[3] + view_proj.m[1],
            view_proj.m[7] + view_proj.m[5],
            view_proj.m[11] + view_proj.m[9]
        ).normalized();
        frustum.planes[Bottom].distance = view_proj.m[15] + view_proj.m[13];

        // Extract near plane
        frustum.planes[Near].normal = Vec3(
            view_proj.m[3] + view_proj.m[2],
            view_proj.m[7] + view_proj.m[6],
            view_proj.m[11] + view_proj.m[10]
        ).normalized();
        frustum.planes[Near].distance = view_proj.m[15] + view_proj.m[14];

        // Extract far plane
        frustum.planes[Far].normal = Vec3(
            view_proj.m[3] - view_proj.m[2],
            view_proj.m[7] - view_proj.m[6],
            view_proj.m[11] - view_proj.m[10]
        ).normalized();
        frustum.planes[Far].distance = view_proj.m[15] - view_proj.m[14];

        return frustum;
    }

    // Check if point is inside frustum
    bool contains(const Vec3& point) const {
        for (int i = 0; i < 6; ++i) {
            if (planes[i].distance_to_point(point) < -EPSILON) {
                return false;
            }
        }
        return true;
    }

    // Check if AABB is inside or intersects frustum
    bool intersects(const AABB& aabb) const {
        Vec3 center = aabb.center();
        Vec3 extents = aabb.extents();

        for (int i = 0; i < 6; ++i) {
            const Plane& plane = planes[i];
            float r = extents.x * std::abs(plane.normal.x) +
                      extents.y * std::abs(plane.normal.y) +
                      extents.z * std::abs(plane.normal.z);
            float distance = plane.distance_to_point(center);

            if (distance < -r) {
                return false;
            }
        }
        return true;
    }

    // Check if sphere is inside or intersects frustum
    bool intersects_sphere(const Vec3& center, float radius) const {
        for (int i = 0; i < 6; ++i) {
            float distance = planes[i].distance_to_point(center);
            if (distance < -radius) {
                return false;
            }
        }
        return true;
    }
};

} // namespace poko::math

#endif // POKO_MATH_FRUSTUM_H
