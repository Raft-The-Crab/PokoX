/**
 * @file collision.h
 * @brief Collision detection system for scene graph.
 *
 * Provides collision detection between entities and shapes.
 *
 * Production Status: M1 World Foundation (Production)
 */

#ifndef POKO_WORLD_COLLISION_H
#define POKO_WORLD_COLLISION_H

#include "poko/math/vec3.h"
#include "poko/math/aabb.h"
#include "poko/math/sphere.h"
#include "poko/math/capsule.h"
#include "poko/math/obb.h"
#include "poko/math/ray.h"
#include "poko/world/entity.h"

namespace poko::world {

/**
 * @brief Collision types.
 */
enum class CollisionType {
    None,
    Trigger,      // Trigger volumes (no physical response)
    Solid        // Solid colliders (physical response)
};

/**
 * @brief Collision shape types.
 */
enum class CollisionShape {
    AABB,
    Sphere,
    Capsule,
    OBB
};

/**
 * @brief Collision layer flags for filtering.
 */
enum class CollisionLayer : uint32_t {
    None = 0,
    Default = 1 << 0,
    Player = 1 << 1,
    Enemy = 1 << 2,
    Environment = 1 << 3,
    Trigger = 1 << 4,
    All = 0xFFFFFFFF
};

/**
 * @brief Collision component - collision shape and properties.
 */
class Collision : public Component {
public:
    Collision();
    ~Collision() override;

    // Non-copyable, non-movable
    Collision(const Collision&) = delete;
    Collision& operator=(const Collision&) = delete;
    Collision(Collision&&) = delete;
    Collision& operator=(Collision&&) = delete;

    /**
     * @brief Initializes the collision component.
     */
    void initialize();

    /**
     * @brief Sets collision type.
     */
    void set_type(CollisionType type);

    /**
     * @brief Gets collision type.
     */
    CollisionType get_type() const;

    /**
     * @brief Sets collision shape as AABB.
     */
    void set_aabb(const math::AABB& aabb);

    /**
     * @brief Gets collision shape as AABB.
     */
    const math::AABB& get_aabb() const;

    /**
     * @brief Sets collision shape as sphere.
     */
    void set_sphere(const math::Sphere& sphere);

    /**
     * @brief Gets collision shape as sphere.
     */
    const math::Sphere& get_sphere() const;

    /**
     * @brief Sets collision shape as capsule.
     */
    void set_capsule(const math::Capsule& capsule);

    /**
     * @brief Gets collision shape as capsule.
     */
    const math::Capsule& get_capsule() const;

    /**
     * @brief Sets collision shape as OBB.
     */
    void set_obb(const math::OBB& obb);

    /**
     * @brief Gets collision shape as OBB.
     */
    const math::OBB& get_obb() const;

    /**
     * @brief Gets the collision shape type.
     */
    CollisionShape get_shape_type() const;

    /**
     * @brief Sets collision layers (what this collider is).
     */
    void set_collision_layers(uint32_t layers);

    /**
     * @brief Gets collision layers.
     */
    uint32_t get_collision_layers() const;

    /**
     * @brief Sets collision mask (what this collider collides with).
     */
    void set_collision_mask(uint32_t mask);

    /**
     * @brief Gets collision mask.
     */
    uint32_t get_collision_mask() const;

    /**
     * @brief Checks if collision with another entity.
     *
     * @param other Other entity to check collision with.
     * @return true if colliding, false otherwise.
     */
    bool check_collision(const Entity* other) const;

    /**
     * @brief Checks if collision with another entity (with layer filtering).
     *
     * @param other Other entity to check collision with.
     * @return true if colliding, false otherwise.
     */
    bool check_collision_filtered(const Entity* other) const;

    /**
     * @brief Gets collision bounds in world space.
     */
    math::AABB get_world_bounds() const;

    /**
     * @brief Gets the entity that owns this component (nullptr if detached).
     */
    Entity* get_entity() const;

    /**
     * @brief Raycasts against this collider in world space.
     *
     * @param ray Ray to cast (world space).
     * @param max_distance Maximum ray distance.
     * @return Hit distance along the ray, or -1.0f on miss.
     */
    float raycast_distance(const math::Ray& ray, float max_distance) const;

    /**
     * @brief Performs raycast against this collider.
     *
     * @param ray Ray to cast (world space).
     * @param max_distance Maximum ray distance.
     * @return true if ray hits, false otherwise.
     */
    bool raycast(const math::Ray& ray, float max_distance) const;

    /**
     * @brief Called when component is added to entity.
     */
    void on_add(Entity* entity) override;

    /**
     * @brief Called when component is removed from entity.
     */
    void on_remove(Entity* entity) override;

private:
    CollisionType type_;
    CollisionShape shape_;
    math::AABB aabb_;
    math::Sphere sphere_;
    math::Capsule capsule_;
    math::OBB obb_;
    uint32_t collision_layers_;
    uint32_t collision_mask_;
    Entity* entity_ = nullptr;
};

/**
 * @brief Collision result - information about collision.
 */
struct CollisionResult {
    bool collided = false;
    math::Vec3 contact_point;
    math::Vec3 normal;
    float penetration_depth = 0.0f;
};

/**
 * @brief Performs collision detection between two entities.
 *
 * @param entity1 First entity.
 * @param entity2 Second entity.
 * @return Collision result.
 */
CollisionResult check_entity_collision(const Entity* entity1, const Entity* entity2);

/**
 * @brief Raycast result against a scene.
 */
struct RaycastResult {
    bool hit = false;
    float distance = 0.0f;
    math::Vec3 point;
    math::Vec3 normal;
    Entity* entity = nullptr;
};

/**
 * @brief Raycasts against all colliders in a scene and returns the closest hit.
 *
 * @param scene Scene to raycast against.
 * @param ray Ray in world space.
 * @param max_distance Maximum ray distance.
 * @param layer_mask Only colliders whose layers intersect this mask are tested.
 * @return Closest raycast hit, or a result with hit == false on miss.
 */
RaycastResult raycast_scene(Scene* scene, const math::Ray& ray, float max_distance,
                            uint32_t layer_mask = 0xFFFFFFFFu);

} // namespace poko::world

#endif // POKO_WORLD_COLLISION_H
