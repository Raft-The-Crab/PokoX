/**
 * @file transform.h
 * @brief Transform struct for position, rotation, and scale.
 */

#ifndef POKO_MATH_TRANSFORM_H
#define POKO_MATH_TRANSFORM_H

#include "vec3.h"
#include "quat.h"
#include "mat4.h"

namespace poko::math {

struct Transform {
    Vec3 position = Vec3::zero();
    Quat rotation = Quat::identity();
    Vec3 scale = Vec3::one();

    Transform() = default;
    Transform(const Vec3& pos, const Quat& rot, const Vec3& scl)
        : position(pos), rotation(rot), scale(scl) {}

    // Get world matrix from transform
    Mat4 to_matrix() const {
        Mat4 scale_mat = Mat4::scale(scale);
        Mat4 rotation_mat = rotation.to_matrix();
        Mat4 translation_mat = Mat4::translation(position);
        return translation_mat * rotation_mat * scale_mat;
    }

    // Transform a point
    Vec3 transform_point(const Vec3& point) const {
        Vec3 rotated = rotation.rotate(point);
        return Vec3(rotated.x * scale.x, rotated.y * scale.y, rotated.z * scale.z) + position;
    }

    // Transform a direction (no translation)
    Vec3 transform_direction(const Vec3& direction) const {
        return rotation.rotate(direction);
    }

    // Identity transform
    static Transform identity() {
        return Transform(Vec3::zero(), Quat::identity(), Vec3::one());
    }
};

} // namespace poko::math

#endif // POKO_MATH_TRANSFORM_H
