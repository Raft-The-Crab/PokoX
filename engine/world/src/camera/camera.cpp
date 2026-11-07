/**
 * @file camera.cpp
 * @brief Camera system implementation.
 *
 * Production Status: M1 World Foundation (Production)
 */

#include "poko/world/camera.h"
#include "poko/math/mat4.h"

namespace poko::world {

Camera::Camera() : dirty_(true) {
    initialize();
}

Camera::~Camera() = default;

void Camera::initialize() {
    type_ = CameraType::Perspective;
    fov_y_ = 45.0f * 3.14159f / 180.0f;  // 45 degrees in radians
    aspect_ = 16.0f / 9.0f;
    near_z_ = 0.1f;
    far_z_ = 1000.0f;
    ortho_left_ = -1.0f;
    ortho_right_ = 1.0f;
    ortho_bottom_ = -1.0f;
    ortho_top_ = 1.0f;
    position_ = math::Vec3(0.0f, 0.0f, 0.0f);
    look_at_ = math::Vec3(0.0f, 0.0f, -1.0f);
    up_vector_ = math::Vec3(0.0f, 1.0f, 0.0f);
    dirty_ = true;
}

void Camera::set_type(CameraType type) {
    type_ = type;
    dirty_ = true;
}

CameraType Camera::get_type() const {
    return type_;
}

void Camera::set_perspective(float fov_y, float aspect, float near_z, float far_z) {
    fov_y_ = fov_y;
    aspect_ = aspect;
    near_z_ = near_z;
    far_z_ = far_z;
    dirty_ = true;
}

void Camera::set_orthographic(float left, float right, float bottom, float top, float near_z, float far_z) {
    ortho_left_ = left;
    ortho_right_ = right;
    ortho_bottom_ = bottom;
    ortho_top_ = top;
    near_z_ = near_z;
    far_z_ = far_z;
    dirty_ = true;
}

math::Mat4 Camera::get_view_matrix() const {
    return math::Mat4::look_at(position_, look_at_, up_vector_);
}

math::Mat4 Camera::get_projection_matrix() const {
    if (type_ == CameraType::Perspective) {
        return math::Mat4::perspective(fov_y_, aspect_, near_z_, far_z_);
    } else {
        return math::Mat4::orthographic(ortho_left_, ortho_right_, ortho_bottom_, ortho_top_, near_z_, far_z_);
    }
}

math::Mat4 Camera::get_view_projection_matrix() const {
    return get_projection_matrix() * get_view_matrix();
}

void Camera::set_position(const math::Vec3& position) {
    position_ = position;
    dirty_ = true;
}

math::Vec3 Camera::get_position() const {
    return position_;
}

void Camera::set_look_at(const math::Vec3& target) {
    look_at_ = target;
    dirty_ = true;
}

math::Vec3 Camera::get_look_at() const {
    return look_at_;
}

void Camera::set_up_vector(const math::Vec3& up) {
    up_vector_ = up;
    dirty_ = true;
}

math::Vec3 Camera::get_up_vector() const {
    return up_vector_;
}

} // namespace poko::world
