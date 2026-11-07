/**
 * @file transform.cpp
 * @brief Poko transform - position, rotation, and scale for entities.
 *
 * Transform represents the local and world space transformation of an entity.
 * It handles parent-child transform propagation for efficient world matrix
 * computation.
 *
 * Production Status: M1 World Foundation
 */

#include "poko/world/transform.h"
#include "poko/math/mat4.h"
#include <algorithm>
#include <cmath>

namespace poko::world {

// Extract a quaternion from a pure rotation matrix (Shepperd's method).
// Matrix layout is column-major: element (row, col) is m[col * 4 + row].
static math::Quat quaternion_from_rotation_matrix(const math::Mat4& m) {
    float r00 = m.m[0],  r01 = m.m[4],  r02 = m.m[8];
    float r10 = m.m[1],  r11 = m.m[5],  r12 = m.m[9];
    float r20 = m.m[2],  r21 = m.m[6],  r22 = m.m[10];

    float trace = r00 + r11 + r22;
    math::Quat q;

    if (trace > 0.0f) {
        float s = std::sqrt(trace + 1.0f) * 2.0f;
        q.w = 0.25f * s;
        q.x = (r21 - r12) / s;
        q.y = (r02 - r20) / s;
        q.z = (r10 - r01) / s;
    } else if (r00 > r11 && r00 > r22) {
        float s = std::sqrt(1.0f + r00 - r11 - r22) * 2.0f;
        q.w = (r21 - r12) / s;
        q.x = 0.25f * s;
        q.y = (r01 + r10) / s;
        q.z = (r02 + r20) / s;
    } else if (r11 > r22) {
        float s = std::sqrt(1.0f + r11 - r00 - r22) * 2.0f;
        q.w = (r02 - r20) / s;
        q.x = (r01 + r10) / s;
        q.y = 0.25f * s;
        q.z = (r12 + r21) / s;
    } else {
        float s = std::sqrt(1.0f + r22 - r00 - r11) * 2.0f;
        q.w = (r10 - r01) / s;
        q.x = (r02 + r20) / s;
        q.y = (r12 + r21) / s;
        q.z = 0.25f * s;
    }

    return q.normalized();
}

// Helper function to convert quaternion to rotation matrix
static math::Mat4 quaternion_to_matrix(const math::Quat& q) {
    // Normalize quaternion
    math::Quat nq = q.normalized();

    float xx = nq.x * nq.x;
    float yy = nq.y * nq.y;
    float zz = nq.z * nq.z;
    float xy = nq.x * nq.y;
    float xz = nq.x * nq.z;
    float yz = nq.y * nq.z;
    float wx = nq.w * nq.x;
    float wy = nq.w * nq.y;
    float wz = nq.w * nq.z;

    math::Mat4 result;

    // Column-major layout
    result.m[0] = 1.0f - 2.0f * (yy + zz);
    result.m[1] = 2.0f * (xy + wz);
    result.m[2] = 2.0f * (xz - wy);
    result.m[3] = 0.0f;

    result.m[4] = 2.0f * (xy - wz);
    result.m[5] = 1.0f - 2.0f * (xx + zz);
    result.m[6] = 2.0f * (yz + wx);
    result.m[7] = 0.0f;

    result.m[8] = 2.0f * (xz + wy);
    result.m[9] = 2.0f * (yz - wx);
    result.m[10] = 1.0f - 2.0f * (xx + yy);
    result.m[11] = 0.0f;

    result.m[12] = 0.0f;
    result.m[13] = 0.0f;
    result.m[14] = 0.0f;
    result.m[15] = 1.0f;

    return result;
}

// -----------------------------------------------------------------------------
// Transform implementation
// -----------------------------------------------------------------------------

Transform::Transform() : dirty_(true), parent_(nullptr) {
    initialize();
}

Transform::~Transform() {
    // Remove from parent's children list
    if (parent_) {
        auto it = std::find(parent_->children_.begin(), parent_->children_.end(), this);
        if (it != parent_->children_.end()) {
            parent_->children_.erase(it);
        }
    }

    // Clear children's parent pointers
    for (auto* child : children_) {
        if (child) {
            child->parent_ = nullptr;
        }
    }
}

void Transform::initialize() {
    position_ = math::Vec3(0.0f, 0.0f, 0.0f);
    rotation_ = math::Quat::identity();
    scale_ = math::Vec3(1.0f, 1.0f, 1.0f);
    world_matrix_ = math::Mat4::identity();
    dirty_ = true;
    parent_ = nullptr;
}

const math::Vec3& Transform::get_position() const {
    return position_;
}

void Transform::set_position(const math::Vec3& position) {
    position_ = position;
    mark_dirty();
}

const math::Quat& Transform::get_rotation() const {
    return rotation_;
}

void Transform::set_rotation(const math::Quat& rotation) {
    rotation_ = rotation;
    mark_dirty();
}

const math::Vec3& Transform::get_scale() const {
    return scale_;
}

void Transform::set_scale(const math::Vec3& scale) {
    scale_ = scale;
    mark_dirty();
}

math::Mat4 Transform::get_local_matrix() const {
    // Create TRS matrix with proper quaternion rotation
    math::Mat4 translation = math::Mat4::translation(position_);
    math::Mat4 rotation = quaternion_to_matrix(rotation_);
    math::Mat4 scaling = math::Mat4::scale(scale_);

    return translation * rotation * scaling;
}

math::Mat4 Transform::get_world_matrix() const {
    if (dirty_) {
        // This is a const method, so we need to compute without caching
        math::Mat4 local = get_local_matrix();
        if (parent_) {
            return parent_->get_world_matrix() * local;
        } else {
            return local;
        }
    }
    return world_matrix_;
}

void Transform::set_from_matrix(const math::Mat4& matrix) {
    // Decompose matrix into TRS. Assumes the matrix has no shear; the basis
    // columns are orthonormalized after scale extraction.

    // Extract translation (column-major: translation is in column 3, indices 12-14)
    position_ = math::Vec3(matrix.m[12], matrix.m[13], matrix.m[14]);

    // Extract scale as the length of each basis column
    math::Vec3 c0(matrix.m[0], matrix.m[1], matrix.m[2]);
    math::Vec3 c1(matrix.m[4], matrix.m[5], matrix.m[6]);
    math::Vec3 c2(matrix.m[8], matrix.m[9], matrix.m[10]);

    float sx = c0.length();
    float sy = c1.length();
    float sz = c2.length();
    scale_ = math::Vec3(sx, sy, sz);

    // Build a pure rotation matrix from the normalized basis columns. Degenerate
    // (zero-length) axes fall back to the identity basis so the quaternion
    // extraction stays well-defined.
    math::Mat4 rot = math::Mat4::identity();
    auto set_column = [&rot](int col, const math::Vec3& axis, float len, const math::Vec3& fallback) {
        math::Vec3 n = (len > math::EPSILON) ? (axis / len) : fallback;
        rot.m[col * 4 + 0] = n.x;
        rot.m[col * 4 + 1] = n.y;
        rot.m[col * 4 + 2] = n.z;
    };
    set_column(0, c0, sx, math::Vec3::right());
    set_column(1, c1, sy, math::Vec3::up());
    set_column(2, c2, sz, math::Vec3::forward());

    rotation_ = quaternion_from_rotation_matrix(rot);

    mark_dirty();
}

void Transform::set_parent(const Transform* parent) {
    // Prevent circular references
    if (parent == this) {
        return;
    }

    // Check if parent is a descendant of this transform
    const Transform* current = parent;
    while (current) {
        if (current == this) {
            return;  // Would create circular reference
        }
        current = current->parent_;
    }

    // Remove from old parent's children
    if (parent_) {
        auto it = std::find(parent_->children_.begin(), parent_->children_.end(), this);
        if (it != parent_->children_.end()) {
            parent_->children_.erase(it);
        }
    }

    // Set new parent (const_cast needed for children management)
    parent_ = const_cast<Transform*>(parent);

    // Add to new parent's children
    if (parent_) {
        parent_->children_.push_back(this);
    }

    mark_dirty();
}

const Transform* Transform::get_parent() const {
    return parent_;
}

math::Vec3 Transform::transform_point(const math::Vec3& local_point) const {
    math::Mat4 world = get_world_matrix();
    math::Vec4 result = world.transform_point(local_point);
    return math::Vec3(result.x, result.y, result.z);
}

math::Vec3 Transform::transform_direction(const math::Vec3& local_direction) const {
    math::Mat4 world = get_world_matrix();
    return world.transform_direction(local_direction);
}

void Transform::mark_dirty() {
    dirty_ = true;
    // Propagate dirty flag to children
    for (auto* child : children_) {
        if (child) {
            child->mark_dirty();
        }
    }
}

bool Transform::is_dirty() const {
    return dirty_;
}

void Transform::update_world_matrix() {
    // Dirty flags propagate parent -> children, so a clean transform's cached
    // world matrix is guaranteed up to date.
    if (!dirty_) {
        return;
    }

    math::Mat4 local = get_local_matrix();

    if (parent_) {
        // Combine with parent's world matrix
        world_matrix_ = parent_->get_world_matrix() * local;
    } else {
        // Root transform - local is world
        world_matrix_ = local;
    }

    dirty_ = false;
}

} // namespace poko::world
