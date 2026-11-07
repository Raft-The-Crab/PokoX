/**
 * @file transform.h
 * @brief Poko transform - position, rotation, and scale for entities.
 *
 * Transform represents the local and world space transformation of an entity.
 * It handles parent-child transform propagation for efficient world matrix
 * computation.
 *
 * Production Status: M1 World Foundation
 */

#ifndef POKO_WORLD_TRANSFORM_H
#define POKO_WORLD_TRANSFORM_H

#include "poko/math/vec3.h"
#include "poko/math/quat.h"
#include "poko/math/mat4.h"
#include "poko/math/vec4.h"
#include <vector>

namespace poko::world {

/**
 * @brief Transform component - position, rotation, scale.
 */
class Transform {
public:
    Transform();
    ~Transform();

    // Non-copyable, non-movable (owned by entity)
    Transform(const Transform&) = delete;
    Transform& operator=(const Transform&) = delete;
    Transform(Transform&&) = delete;
    Transform& operator=(Transform&&) = delete;

    /**
     * @brief Initializes the transform to identity.
     */
    void initialize();

    /**
     * @brief Gets the local position.
     */
    const math::Vec3& get_position() const;

    /**
     * @brief Sets the local position.
     *
     * @param position New position.
     */
    void set_position(const math::Vec3& position);

    /**
     * @brief Gets the local rotation.
     */
    const math::Quat& get_rotation() const;

    /**
     * @brief Sets the local rotation.
     *
     * @param rotation New rotation.
     */
    void set_rotation(const math::Quat& rotation);

    /**
     * @brief Gets the local scale.
     */
    const math::Vec3& get_scale() const;

    /**
     * @brief Sets the local scale.
     *
     * @param scale New scale.
     */
    void set_scale(const math::Vec3& scale);

    /**
     * @brief Gets the local transform matrix (TRS).
     */
    math::Mat4 get_local_matrix() const;

    /**
     * @brief Gets the world transform matrix.
     *
     * This computes the world matrix by combining local transform with
     * parent's world transform (if parent exists).
     */
    math::Mat4 get_world_matrix() const;

    /**
     * @brief Sets the transform from a matrix (decomposes TRS).
     *
     * @param matrix Transformation matrix.
     */
    void set_from_matrix(const math::Mat4& matrix);

    /**
     * @brief Sets the parent transform for hierarchy.
     *
     * @param parent Parent transform (nullptr for root).
     */
    void set_parent(const Transform* parent);

    /**
     * @brief Gets the parent transform.
     */
    const Transform* get_parent() const;

    /**
     * @brief Marks the transform as dirty (needs world matrix recompute).
     *
     * This also marks all children as dirty for propagation.
     */
    void mark_dirty();

    /**
     * @brief Checks if the transform is dirty.
     */
    bool is_dirty() const;

    /**
     * @brief Updates the world matrix (called during world update).
     */
    void update_world_matrix();

    /**
     * @brief Transforms a local point to world space.
     *
     * @param local_point Point in local space.
     * @return Point in world space.
     */
    math::Vec3 transform_point(const math::Vec3& local_point) const;

    /**
     * @brief Transforms a local direction to world space.
     *
     * @param local_direction Direction in local space.
     * @return Direction in world space.
     */
    math::Vec3 transform_direction(const math::Vec3& local_direction) const;

private:
    math::Vec3 position_;
    math::Quat rotation_;
    math::Vec3 scale_;
    math::Mat4 world_matrix_;
    bool dirty_;
    Transform* parent_;  // Non-const for children management
    std::vector<Transform*> children_;  // For dirty propagation

    // Need forward declaration for destructor
    friend class Entity;
};

} // namespace poko::world

#endif // POKO_WORLD_TRANSFORM_H
