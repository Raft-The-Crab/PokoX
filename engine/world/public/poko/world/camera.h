/**
 * @file camera.h
 * @brief Camera system for scene graph rendering.
 *
 * Provides camera functionality for view and projection matrices.
 *
 * Production Status: M1 World Foundation (Production)
 */

#ifndef POKO_WORLD_CAMERA_H
#define POKO_WORLD_CAMERA_H

#include "poko/math/vec3.h"
#include "poko/math/mat4.h"
#include "poko/world/transform.h"

namespace poko::world {

/**
 * @brief Camera types.
 */
enum class CameraType {
    Perspective,
    Orthographic
};

/**
 * @brief Camera component - view and projection matrices.
 */
class Camera {
public:
    Camera();
    ~Camera();

    // Non-copyable, non-movable
    Camera(const Camera&) = delete;
    Camera& operator=(const Camera&) = delete;
    Camera(Camera&&) = delete;
    Camera& operator=(Camera&&) = delete;

    /**
     * @brief Initializes the camera.
     */
    void initialize();

    /**
     * @brief Sets camera type.
     */
    void set_type(CameraType type);

    /**
     * @brief Gets camera type.
     */
    CameraType get_type() const;

    /**
     * @brief Sets perspective camera parameters.
     *
     * @param fov_y Vertical field of view (radians).
     * @param aspect Aspect ratio (width/height).
     * @param near_z Near clipping plane.
     * @param far_z Far clipping plane.
     */
    void set_perspective(float fov_y, float aspect, float near_z, float far_z);

    /**
     * @brief Sets orthographic camera parameters.
     *
     * @param left Left clipping plane.
     * @param right Right clipping plane.
     * @param bottom Bottom clipping plane.
     * @param top Top clipping plane.
     * @param near_z Near clipping plane.
     * @param far_z Far clipping plane.
     */
    void set_orthographic(float left, float right, float bottom, float top, float near_z, float far_z);

    /**
     * @brief Gets the view matrix.
     */
    math::Mat4 get_view_matrix() const;

    /**
     * @brief Gets the projection matrix.
     */
    math::Mat4 get_projection_matrix() const;

    /**
     * @brief Gets the view-projection matrix.
     */
    math::Mat4 get_view_projection_matrix() const;

    /**
     * @brief Sets the camera's world position.
     */
    void set_position(const math::Vec3& position);

    /**
     * @brief Gets the camera's world position.
     */
    math::Vec3 get_position() const;

    /**
     * @brief Sets the camera's look-at target.
     */
    void set_look_at(const math::Vec3& target);

    /**
     * @brief Gets the camera's look-at target.
     */
    math::Vec3 get_look_at() const;

    /**
     * @brief Sets the camera's up vector.
     */
    void set_up_vector(const math::Vec3& up);

    /**
     * @brief Gets the camera's up vector.
     */
    math::Vec3 get_up_vector() const;

private:
    CameraType type_;
    float fov_y_;
    float aspect_;
    float near_z_;
    float far_z_;
    float ortho_left_, ortho_right_, ortho_bottom_, ortho_top_;
    math::Vec3 position_;
    math::Vec3 look_at_;
    math::Vec3 up_vector_;
    bool dirty_;
};

} // namespace poko::world

#endif // POKO_WORLD_CAMERA_H
