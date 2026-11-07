/**
 * @file character.h
 * @brief Character controller for scene graph.
 *
 * Provides character movement and physics for entities.
 *
 * Production Status: M1 World Foundation (Production)
 */

#ifndef POKO_WORLD_CHARACTER_H
#define POKO_WORLD_CHARACTER_H

#include "poko/math/vec3.h"
#include "poko/world/component.h"
#include "poko/world/entity.h"

namespace poko::world {

/**
 * @brief Character controller component - movement and physics.
 */
class CharacterController : public Component {
public:
    CharacterController();
    ~CharacterController() override;

    // Non-copyable, non-movable
    CharacterController(const CharacterController&) = delete;
    CharacterController& operator=(const CharacterController&) = delete;
    CharacterController(CharacterController&&) = delete;
    CharacterController& operator=(CharacterController&&) = delete;

    /**
     * @brief Initializes the character controller.
     */
    void initialize();

    /**
     * @brief Sets movement speed.
     */
    void set_move_speed(float speed);

    /**
     * @brief Gets movement speed.
     */
    float get_move_speed() const;

    /**
     * @brief Sets jump velocity.
     */
    void set_jump_velocity(float velocity);

    /**
     * @brief Gets jump velocity.
     */
    float get_jump_velocity() const;

    /**
     * @brief Sets gravity.
     */
    void set_gravity(float gravity);

    /**
     * @brief Gets gravity.
     */
    float get_gravity() const;

    /**
     * @brief Sets the character's velocity.
     */
    void set_velocity(const math::Vec3& velocity);

    /**
     * @brief Gets the character's velocity.
     */
    math::Vec3 get_velocity() const;

    /**
     * @brief Moves the character.
     *
     * @param direction Movement direction (normalized).
     * @param delta_time Time since last update (seconds).
     */
    void move(const math::Vec3& direction, float delta_time);

    /**
     * @brief Moves the character with acceleration.
     *
     * @param direction Movement direction (normalized).
     * @param delta_time Time since last update (seconds).
     * @param acceleration Acceleration rate.
     */
    void move_with_acceleration(const math::Vec3& direction, float delta_time, float acceleration);

    /**
     * @brief Makes the character jump.
     */
    void jump();

    /**
     * @brief Checks if the character is grounded.
     */
    bool is_grounded() const;

    /**
     * @brief Sets grounded state.
     */
    void set_grounded(bool grounded);

    /**
     * @brief Sets the character's facing direction.
     */
    void set_facing_direction(const math::Vec3& direction);

    /**
     * @brief Gets the character's facing direction.
     */
    math::Vec3 get_facing_direction() const;

    /**
     * @brief Sets the ground height used for grounded detection.
     */
    void set_ground_height(float height);

    /**
     * @brief Gets the ground height.
     */
    float get_ground_height() const;

    /**
     * @brief Sets the maximum fall speed (positive magnitude; 0 disables).
     */
    void set_max_fall_speed(float speed);

    /**
     * @brief Gets the maximum fall speed.
     */
    float get_max_fall_speed() const;

    /**
     * @brief Enables/disables collision-based ground detection.
     *
     * When enabled, the controller raycasts downward each frame and grounds
     * on the first solid collider hit instead of the flat ground_height_
     * plane. The plane check remains as a fallback.
     */
    void set_collision_grounding(bool enabled);

    /**
     * @brief Returns true if collision-based ground detection is enabled.
     */
    bool get_collision_grounding() const;

    /**
     * @brief Sets the ground probe distance (how far below the entity the
     * ground ray reaches, in meters).
     */
    void set_ground_probe_distance(float distance);

    /**
     * @brief Gets the ground probe distance.
     */
    float get_ground_probe_distance() const;

    /**
     * @brief Sets the collision layers the ground probe tests against.
     */
    void set_ground_probe_mask(uint32_t mask);

    /**
     * @brief Gets the ground probe mask.
     */
    uint32_t get_ground_probe_mask() const;

    /**
     * @brief Updates the character controller.
     *
     * @param delta_time Time since last update (seconds).
     */
    void on_update(float delta_time) override;

    /**
     * @brief Called when component is added to entity.
     */
    void on_add(Entity* entity) override;

    /**
     * @brief Called when component is removed from entity.
     */
    void on_remove(Entity* entity) override;

private:
    float move_speed_;
    float jump_velocity_;
    float gravity_;
    math::Vec3 velocity_;
    bool grounded_;
    Entity* entity_;
    math::Vec3 facing_direction_;
    float acceleration_;
    float deceleration_;
    float ground_height_;
    float max_fall_speed_;
    bool accel_input_ = false;
    bool collision_grounding_ = false;
    float ground_probe_distance_ = 0.25f;
    uint32_t ground_probe_mask_ = 0xFFFFFFFFu;
};

} // namespace poko::world

#endif // POKO_WORLD_CHARACTER_H
