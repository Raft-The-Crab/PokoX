/**
 * @file test_math.cpp
 * @brief Unit tests for PokoX math library.
 */

#include "poko/math/math.h"

#include <cstdio>
#include <cassert>
#include <cmath>
#include <limits>

int main() {
    std::printf("PokoX Math Library Tests\n");
    std::printf("=========================\n\n");

    // Constants tests
    {
        std::printf("Constants tests...\n");
        assert(poko::math::approximately_equal(1.0f, 1.0f));
        assert(poko::math::approximately_equal(1.0f, 1.000001f));
        assert(!poko::math::approximately_equal(1.0f, 1.1f));
        assert(poko::math::approximately_zero(0.0f));
        assert(poko::math::approximately_zero(0.0000001f));
        std::printf("  PASS\n");
    }

    // Vec2 tests
    {
        std::printf("Vec2 tests...\n");
        poko::math::Vec2 a(1.0f, 2.0f);
        poko::math::Vec2 b(3.0f, 4.0f);
        
        poko::math::Vec2 c = a + b;
        assert(c.x == 4.0f && c.y == 6.0f);
        
        float dot = a.dot(b);
        assert(dot == 11.0f);
        
        float len = a.length();
        assert(std::abs(len - std::sqrt(5.0f)) < 0.001f);
        
        // Test lerp
        auto lerped = poko::math::Vec2::lerp(a, b, 0.5f);
        assert(std::abs(lerped.x - 2.0f) < 0.001f);
        assert(std::abs(lerped.y - 3.0f) < 0.001f);
        
        std::printf("  PASS\n");
    }

    // Vec3 tests
    {
        std::printf("Vec3 tests...\n");
        poko::math::Vec3 a(1.0f, 0.0f, 0.0f);
        poko::math::Vec3 b(0.0f, 1.0f, 0.0f);
        
        poko::math::Vec3 cross = a.cross(b);
        assert(std::abs(cross.x) < 0.001f);
        assert(std::abs(cross.y) < 0.001f);
        assert(std::abs(cross.z - 1.0f) < 0.001f);
        
        float dot = a.dot(b);
        assert(dot == 0.0f);
        
        // Test lerp
        auto lerped = poko::math::Vec3::lerp(a, b, 0.5f);
        assert(std::abs(lerped.x - 0.5f) < 0.001f);
        assert(std::abs(lerped.y - 0.5f) < 0.001f);
        
        std::printf("  PASS\n");
    }

    // Vec4 tests
    {
        std::printf("Vec4 tests...\n");
        poko::math::Vec4 a(1.0f, 2.0f, 3.0f, 4.0f);
        poko::math::Vec4 b(5.0f, 6.0f, 7.0f, 8.0f);
        
        float dot = a.dot(b);
        assert(dot == 70.0f);
        
        // Test lerp
        auto lerped = poko::math::Vec4::lerp(a, b, 0.5f);
        assert(std::abs(lerped.x - 3.0f) < 0.001f);
        assert(std::abs(lerped.y - 4.0f) < 0.001f);
        
        std::printf("  PASS\n");
    }

    // Mat4 tests
    {
        std::printf("Mat4 tests...\n");
        auto identity = poko::math::Mat4::identity();
        assert(identity.m[0] == 1.0f && identity.m[5] == 1.0f && identity.m[10] == 1.0f && identity.m[15] == 1.0f);
        
        auto trans = poko::math::Mat4::translation(1.0f, 2.0f, 3.0f);
        assert(trans.m[12] == 1.0f && trans.m[13] == 2.0f && trans.m[14] == 3.0f);
        
        auto scale = poko::math::Mat4::scale(2.0f, 3.0f, 4.0f);
        assert(scale.m[0] == 2.0f && scale.m[5] == 3.0f && scale.m[10] == 4.0f);
        
        // Test matrix multiplication with identity
        auto multiplied = identity * trans;
        assert(std::abs(multiplied.m[12] - 1.0f) < 0.001f);
        
        // Test transform_point
        auto transform = poko::math::Mat4::translation(5.0f, 0.0f, 0.0f);
        auto point = transform.transform_point(poko::math::Vec3(1.0f, 2.0f, 3.0f));
        assert(std::abs(point.x - 6.0f) < 0.001f);
        assert(std::abs(point.y - 2.0f) < 0.001f);
        assert(std::abs(point.z - 3.0f) < 0.001f);
        
        // Test transform_direction (no translation)
        auto dir = transform.transform_direction(poko::math::Vec3(1.0f, 0.0f, 0.0f));
        assert(std::abs(dir.x - 1.0f) < 0.001f);
        assert(std::abs(dir.y - 0.0f) < 0.001f);
        assert(std::abs(dir.z - 0.0f) < 0.001f);
        
        // Test scale on point
        auto scale_mat = poko::math::Mat4::scale(2.0f, 2.0f, 2.0f);
        auto scaled_point = scale_mat.transform_point(poko::math::Vec3(1.0f, 2.0f, 3.0f));
        assert(std::abs(scaled_point.x - 2.0f) < 0.001f);
        assert(std::abs(scaled_point.y - 4.0f) < 0.001f);
        assert(std::abs(scaled_point.z - 6.0f) < 0.001f);
        
        // Test rotation Z (90 degrees)
        auto rot_z = poko::math::Mat4::rotation_z(1.5708f); // ~90 degrees
        auto rotated = rot_z.transform_point(poko::math::Vec3(1.0f, 0.0f, 0.0f));
        assert(std::abs(rotated.x) < 0.1f); // Should be near 0
        assert(std::abs(rotated.y - 1.0f) < 0.1f); // Should be near 1
        
        // Test transpose
        auto transposed = trans.transpose();
        assert(std::abs(transposed.m[0] - 1.0f) < 0.001f);
        assert(std::abs(transposed.m[1] - 0.0f) < 0.001f); // Translation moves to bottom row
        assert(std::abs(transposed.m[12] - 0.0f) < 0.001f);
        
        // Test determinant (identity should be 1)
        assert(std::abs(identity.determinant() - 1.0f) < 0.001f);
        
        // Test inverse (identity inverse is identity)
        auto inv = identity.inverse();
        assert(std::abs(inv.m[0] - 1.0f) < 0.001f);
        
        // Test lerp
        auto lerped = poko::math::Mat4::lerp(identity, trans, 0.5f);
        assert(std::abs(lerped.m[12] - 0.5f) < 0.001f);
        
        std::printf("  PASS\n");
    }

    // Quat tests
    {
        std::printf("Quat tests...\n");
        auto identity = poko::math::Quat::identity();
        assert(identity.w == 1.0f);
        
        auto q = poko::math::Quat::from_axis_angle(poko::math::Vec3::up(), 3.14159f);
        assert(std::abs(q.w) < 0.1f); // 180 degree rotation
        
        // Test quaternion to matrix
        auto rot_mat = q.to_matrix();
        assert(rot_mat.m[15] == 1.0f); // Rotation matrices have w=1
        
        // Test quaternion rotation
        auto forward = q.rotate(poko::math::Vec3::forward());
        assert(std::abs(forward.x) < 0.1f); // Forward should rotate to -Z
        
        // Test quaternion multiplication
        auto q1 = poko::math::Quat::from_axis_angle(poko::math::Vec3::up(), 1.0f);
        auto q2 = poko::math::Quat::from_axis_angle(poko::math::Vec3::up(), 1.0f);
        auto q_combined = q1 * q2;
        assert(q_combined.length() > 0.9f && q_combined.length() < 1.1f); // Should be normalized
        
        // Test quaternion slerp
        auto q_start = poko::math::Quat::identity();
        auto q_end = poko::math::Quat::from_axis_angle(poko::math::Vec3::up(), 1.57f);
        auto q_mid = poko::math::Quat::slerp(q_start, q_end, 0.5f);
        assert(q_mid.length() > 0.9f && q_mid.length() < 1.1f); // Should be normalized
        
        std::printf("  PASS\n");
    }

    // Transform tests
    {
        std::printf("Transform tests...\n");
        auto identity = poko::math::Transform::identity();
        assert(identity.position.is_zero());
        assert(identity.scale.x == 1.0f);
        
        // Test transform with position
        poko::math::Transform t(
            poko::math::Vec3(10.0f, 0.0f, 0.0f),
            poko::math::Quat::identity(),
            poko::math::Vec3::one()
        );
        
        auto world_mat = t.to_matrix();
        assert(std::abs(world_mat.m[12] - 10.0f) < 0.001f);
        
        // Test transform with scale
        poko::math::Transform t_scale(
            poko::math::Vec3::zero(),
            poko::math::Quat::identity(),
            poko::math::Vec3(2.0f, 2.0f, 2.0f)
        );
        
        auto scaled_mat = t_scale.to_matrix();
        assert(std::abs(scaled_mat.m[0] - 2.0f) < 0.001f);
        assert(std::abs(scaled_mat.m[5] - 2.0f) < 0.001f);
        assert(std::abs(scaled_mat.m[10] - 2.0f) < 0.001f);
        
        // Test transform_point
        auto t2 = poko::math::Transform(
            poko::math::Vec3(5.0f, 0.0f, 0.0f),
            poko::math::Quat::identity(),
            poko::math::Vec3(2.0f, 1.0f, 1.0f)
        );
        
        auto transformed = t2.transform_point(poko::math::Vec3(1.0f, 0.0f, 0.0f));
        assert(std::abs(transformed.x - 7.0f) < 0.001f); // (1 * 2) + 5 = 7
        
        std::printf("  PASS\n");
    }

    // AABB tests
    {
        std::printf("AABB tests...\n");
        poko::math::AABB aabb(
            poko::math::Vec3(0.0f, 0.0f, 0.0f),
            poko::math::Vec3(1.0f, 1.0f, 1.0f)
        );
        
        // Test contains
        assert(aabb.contains(poko::math::Vec3(0.5f, 0.5f, 0.5f)));
        assert(!aabb.contains(poko::math::Vec3(2.0f, 2.0f, 2.0f)));
        
        // Test center
        auto center = aabb.center();
        assert(std::abs(center.x - 0.5f) < 0.001f);
        
        // Test size
        auto size = aabb.size();
        assert(std::abs(size.x - 1.0f) < 0.001f);
        
        // Test intersects
        poko::math::AABB other(
            poko::math::Vec3(0.5f, 0.5f, 0.5f),
            poko::math::Vec3(1.5f, 1.5f, 1.5f)
        );
        assert(aabb.intersects(other));
        
        std::printf("  PASS\n");
    }

    // Plane tests
    {
        std::printf("Plane tests...\n");
        auto plane = poko::math::Plane::from_point_normal(
            poko::math::Vec3(0.0f, 0.0f, 0.0f),
            poko::math::Vec3::up()
        );
        
        // Test distance
        float dist = plane.distance_to_point(poko::math::Vec3(0.0f, 1.0f, 0.0f));
        assert(std::abs(dist - 1.0f) < 0.001f);
        
        // Test contains
        assert(plane.contains(poko::math::Vec3(0.0f, 0.0f, 0.0f)));
        
        std::printf("  PASS\n");
    }

    // Ray tests
    {
        std::printf("Ray tests...\n");
        poko::math::Ray ray(
            poko::math::Vec3(0.0f, 0.0f, 0.0f),
            poko::math::Vec3::forward()
        );
        
        // Test point_at
        auto point = ray.point_at(5.0f);
        assert(std::abs(point.z - 5.0f) < 0.001f);
        
        // Test AABB intersection
        poko::math::AABB aabb(
            poko::math::Vec3(0.0f, 0.0f, 1.0f),
            poko::math::Vec3(1.0f, 1.0f, 2.0f)
        );
        float t = ray.intersect_aabb(aabb);
        assert(t > 0.0f);
        
        std::printf("  PASS\n");
    }

    // Frustum tests
    {
        std::printf("Frustum tests...\n");
        auto proj = poko::math::Mat4::perspective(1.57f, 1.0f, 0.1f, 100.0f);
        auto view = poko::math::Mat4::look_at(
            poko::math::Vec3(0.0f, 0.0f, 5.0f),
            poko::math::Vec3(0.0f, 0.0f, 0.0f),
            poko::math::Vec3::up()
        );
        auto frustum = poko::math::Frustum::from_view_projection(proj * view);
        
        // Test contains (origin should be in view)
        assert(frustum.contains(poko::math::Vec3(0.0f, 0.0f, 0.0f)));
        
        // Test AABB intersection
        poko::math::AABB aabb(
            poko::math::Vec3(-1.0f, -1.0f, -1.0f),
            poko::math::Vec3(1.0f, 1.0f, 1.0f)
        );
        assert(frustum.intersects(aabb));
        
        std::printf("  PASS\n");
    }

    std::printf("\nAll math library tests passed.\n");
    return 0;
}
