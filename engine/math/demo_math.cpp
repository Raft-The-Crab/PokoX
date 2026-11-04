/**
 * @file demo_math.cpp
 * @brief Demo for PokoX math library.
 */

#include "poko/math/math.h"

#include <cstdio>

int main() {
    std::printf("PokoX Math Library Demo\n");
    std::printf("======================\n\n");

    // Vector operations
    std::printf("Vector Operations:\n");
    poko::math::Vec3 position(1.0f, 2.0f, 3.0f);
    poko::math::Vec3 velocity(0.1f, 0.2f, 0.0f);
    
    position += velocity;
    std::printf("  Position after move: (%.2f, %.2f, %.2f)\n", position.x, position.y, position.z);
    
    float distance = position.distance_to(poko::math::Vec3::zero());
    std::printf("  Distance from origin: %.2f\n", distance);

    // Matrix operations
    std::printf("\nMatrix Operations:\n");
    auto translation = poko::math::Mat4::translation(5.0f, 0.0f, 0.0f);
    auto scale = poko::math::Mat4::scale(2.0f, 2.0f, 2.0f);
    
    auto combined = translation * scale;
    std::printf("  Combined translation+scale matrix created\n");

    // Quaternion operations
    std::printf("\nQuaternion Operations:\n");
    auto rotation = poko::math::Quat::from_euler(0.0f, 1.57f, 0.0f); // 90 degrees Y
    poko::math::Vec3 forward = rotation.rotate(poko::math::Vec3::forward());
    std::printf("  Rotated forward vector: (%.2f, %.2f, %.2f)\n", forward.x, forward.y, forward.z);

    // Transform operations
    std::printf("\nTransform Operations:\n");
    poko::math::Transform transform(
        poko::math::Vec3(10.0f, 0.0f, 0.0f),
        rotation,
        poko::math::Vec3::one()
    );
    
    auto world_matrix = transform.to_matrix();
    std::printf("  Transform world matrix created\n");

    std::printf("\nDemo completed successfully.\n");
    return 0;
}
