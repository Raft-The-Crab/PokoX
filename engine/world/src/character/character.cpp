/**
 * @file character.cpp
 * @brief Character controller implementation.
 *
 * Production Status: M1 World Foundation (Production)
 */

#include "poko/world/character.h"
#include "poko/world/collision.h"
#include "poko/world/entity.h"
#include "poko/world/scene.h"
#include "poko/world/transform.h"
#include "poko/math/vec3.h"
#include "poko/math/ray.h"

namespace poko::world {

CharacterController::CharacterController() : grounded_(false), entity_(nullptr) {
    initialize();
}

CharacterController::~CharacterController() = default;

void CharacterController::initialize() {
    move_speed_ = 5.0f;
    jump_velocity_ = 8.0f;
    gravity_ = -9.81f;
    velocity_ = math::Vec3(0.0f, 0.0f, 0.0f);
    grounded_ = false;
    entity_ = nullptr;
    facing_direction_ = math::Vec3::forward();
    acceleration_ = 10.0f;
    deceleration_ = 15.0f;
    ground_height_ = 0.0f;
    max_fall_speed_ = 50.0f;
    accel_input_ = false;
    collision_grounding_ = false;
    ground_probe_distance_ = 0.25f;
    ground_probe_mask_ = 0xFFFFFFFFu;
}

void CharacterController::set_move_speed(float speed) {
    move_speed_ = speed;
}

float CharacterController::get_move_speed() const {
    return move_speed_;
}

void CharacterController::set_jump_velocity(float velocity) {
    jump_velocity_ = velocity;
}

float CharacterController::get_jump_velocity() const {
    return jump_velocity_;
}

void CharacterController::set_gravity(float gravity) {
    gravity_ = gravity;
}

float CharacterController::get_gravity() const {
    return gravity_;
}

void CharacterController::set_velocity(const math::Vec3& velocity) {
    velocity_ = velocity;
}

math::Vec3 CharacterController::get_velocity() const {
    return velocity_;
}

void CharacterController::move(const math::Vec3& direction, float delta_time) {
    if (!entity_) {
        return;
    }

    Transform* transform = entity_->get_transform();
    if (!transform) {
        return;
    }

    // Apply movement
    math::Vec3 current_pos = transform->get_position();
    math::Vec3 movement = direction * move_speed_ * delta_time;
    math::Vec3 new_pos = current_pos + movement;

    transform->set_position(new_pos);

    // Update facing direction
    if (!direction.is_zero()) {
        facing_direction_ = direction.normalized();
    }
}

void CharacterController::move_with_acceleration(const math::Vec3& direction, float delta_time, float acceleration) {
    if (!entity_) {
        return;
    }

    // Update velocity only; position integration happens once per frame in
    // on_update(). Applying the displacement here as well would move the
    // character twice per frame.
    math::Vec3 target_velocity = direction * move_speed_;

    math::Vec3 velocity_diff = target_velocity - velocity_;
    float velocity_diff_length = velocity_diff.length();

    if (velocity_diff_length > 0.001f) {
        float max_change = acceleration * delta_time;
        math::Vec3 velocity_change = velocity_diff.normalized() * std::min(max_change, velocity_diff_length);
        velocity_ = velocity_ + velocity_change;
    } else {
        velocity_ = target_velocity;
    }

    accel_input_ = true;

    // Update facing direction
    if (!direction.is_zero()) {
        facing_direction_ = direction.normalized();
    }
}

void CharacterController::jump() {
    if (!grounded_) {
        return;
    }

    velocity_.y = jump_velocity_;
    grounded_ = false;
}

bool CharacterController::is_grounded() const {
    return grounded_;
}

void CharacterController::set_grounded(bool grounded) {
    grounded_ = grounded;
}

void CharacterController::set_facing_direction(const math::Vec3& direction) {
    if (!direction.is_zero()) {
        facing_direction_ = direction.normalized();
    }
}

void CharacterController::set_ground_height(float height) {
    ground_height_ = height;
}

float CharacterController::get_ground_height() const {
    return ground_height_;
}

void CharacterController::set_max_fall_speed(float speed) {
    max_fall_speed_ = (speed > 0.0f) ? speed : 0.0f;
}

float CharacterController::get_max_fall_speed() const {
    return max_fall_speed_;
}

void CharacterController::set_collision_grounding(bool enabled) {
    collision_grounding_ = enabled;
}

bool CharacterController::get_collision_grounding() const {
    return collision_grounding_;
}

void CharacterController::set_ground_probe_distance(float distance) {
    ground_probe_distance_ = (distance > 0.0f) ? distance : 0.0f;
}

float CharacterController::get_ground_probe_distance() const {
    return ground_probe_distance_;
}

void CharacterController::set_ground_probe_mask(uint32_t mask) {
    ground_probe_mask_ = mask;
}

uint32_t CharacterController::get_ground_probe_mask() const {
    return ground_probe_mask_;
}

math::Vec3 CharacterController::get_facing_direction() const {
    return facing_direction_;
}

void CharacterController::on_update(float delta_time) {
    if (!entity_) {
        return;
    }

    Transform* transform = entity_->get_transform();
    if (!transform) {
        return;
    }

    // Consume the input flag for this frame: ground friction only applies to
    // residual velocity when no acceleration input was requested.
    const bool has_move_input = accel_input_;
    accel_input_ = false;

    if (!grounded_) {
        // Apply gravity with an optional terminal fall speed
        velocity_.y += gravity_ * delta_time;
        if (max_fall_speed_ > 0.0f && velocity_.y < -max_fall_speed_) {
            velocity_.y = -max_fall_speed_;
        }
    } else if (!has_move_input) {
        // Apply ground friction when grounded and no move input this frame
        math::Vec3 horizontal_velocity = math::Vec3(velocity_.x, 0.0f, velocity_.z);
        float horizontal_speed = horizontal_velocity.length();
        if (horizontal_speed > 0.001f) {
            float friction = deceleration_ * delta_time;
            float new_speed = std::max(0.0f, horizontal_speed - friction);
            horizontal_velocity = horizontal_velocity.normalized() * new_speed;
            velocity_.x = horizontal_velocity.x;
            velocity_.z = horizontal_velocity.z;
        }
    }

    // Integrate velocity into position exactly once per frame
    math::Vec3 current_pos = transform->get_position();
    math::Vec3 new_pos = current_pos + velocity_ * delta_time;

    // Ground detection: collision raycast when enabled, flat plane otherwise.
    // The probe only grounds the character while it is not moving upward, so
    // jumping is not cancelled by the ground snap.
    bool landed = false;
    float ground_y = ground_height_;

    if (collision_grounding_ && velocity_.y <= 0.0f) {
        if (Scene* scene = entity_->get_scene()) {
            // Probe downward from just above the entity's feet
            math::Ray probe(math::Vec3(new_pos.x, new_pos.y + ground_probe_distance_, new_pos.z),
                            math::Vec3::down());
            RaycastResult hit = raycast_scene(scene, probe,
                                              ground_probe_distance_ * 2.0f,
                                              ground_probe_mask_);
            // Ignore hits on the entity itself; surface must be at/below feet
            if (hit.hit && hit.entity != entity_ && hit.point.y <= new_pos.y + math::EPSILON) {
                landed = true;
                ground_y = hit.point.y;
            }
        }
    }

    if (!landed && velocity_.y <= 0.0f && new_pos.y <= ground_height_) {
        landed = true;
        ground_y = ground_height_;
    }

    if (landed) {
        new_pos.y = ground_y;
        if (velocity_.y < 0.0f) {
            velocity_.y = 0.0f;
        }
        grounded_ = true;
    } else {
        grounded_ = false;
    }

    transform->set_position(new_pos);
}

void CharacterController::on_add(Entity* entity) {
    entity_ = entity;
}

void CharacterController::on_remove(Entity* entity) {
    if (entity_ == entity) {
        entity_ = nullptr;
    }
}

} // namespace poko::world
