/**
 * @file test_world.cpp
 * @brief World module unit tests.
 *
 * Tests the basic functionality of the world/scene/entity/transform system.
 *
 * Production Status: M1 World Foundation
 */

#include "poko/world/world.h"
#include "poko/world/scene.h"
#include "poko/world/entity.h"
#include "poko/world/transform.h"
#include "poko/world/component.h"
#include "poko/world/collision.h"
#include "poko/world/asset.h"
#include "poko/world/character.h"
#include "poko/math/vec3.h"
#include "poko/math/aabb.h"
#include "poko/math/sphere.h"
#include "poko/math/ray.h"
#include "poko/math/mat4.h"
#include "poko/math/constants.h"

#include <iostream>
#include <fstream>
#include <cstdio>
#include <cassert>

using namespace poko::world;
using namespace poko::math;

// Test component for component system tests
struct TestComponent : Component {
    int value = 42;
};

int main() {
    std::cout << "=== PokoX World Module Tests ===\n\n";

    // Test 1: World creation and initialization
    std::cout << "Test 1: World creation and initialization... ";
    World world;
    WorldConfig config;
    config.max_entities = 1000;
    config.max_scenes = 10;

    if (!world.initialize(config)) {
        std::cout << "FAILED\n";
        return 1;
    }
    if (!world.is_valid()) {
        std::cout << "FAILED\n";
        return 1;
    }
    std::cout << "PASSED\n";

    // Test 2: Scene creation
    std::cout << "Test 2: Scene creation... ";
    Scene* scene = world.create_scene("TestScene");
    if (!scene) {
        std::cout << "FAILED\n";
        return 1;
    }
    if (!scene->is_valid()) {
        std::cout << "FAILED\n";
        return 1;
    }
    if (scene->get_name() != "TestScene") {
        std::cout << "FAILED\n";
        return 1;
    }
    std::cout << "PASSED\n";

    // Test 3: Entity creation
    std::cout << "Test 3: Entity creation... ";
    Entity* entity = scene->create_entity();
    if (!entity) {
        std::cout << "FAILED\n";
        return 1;
    }
    if (!entity->is_valid()) {
        std::cout << "FAILED\n";
        return 1;
    }
    std::cout << "PASSED\n";

    // Test 4: Entity transform
    std::cout << "Test 4: Entity transform... ";
    Transform* transform = entity->get_transform();
    if (!transform) {
        std::cout << "FAILED\n";
        return 1;
    }

    // Set position
    Vec3 position(1.0f, 2.0f, 3.0f);
    transform->set_position(position);
    Vec3 actual_pos = transform->get_position();
    if (actual_pos.x != position.x || actual_pos.y != position.y || actual_pos.z != position.z) {
        std::cout << "FAILED\n";
        return 1;
    }

    // Set scale
    Vec3 scale(2.0f, 2.0f, 2.0f);
    transform->set_scale(scale);
    Vec3 actual_scale = transform->get_scale();
    if (actual_scale.x != scale.x || actual_scale.y != scale.y || actual_scale.z != scale.z) {
        std::cout << "FAILED\n";
        return 1;
    }
    std::cout << "PASSED\n";

    // Test 5: Entity hierarchy
    std::cout << "Test 5: Entity hierarchy... ";
    Entity* parent_entity = scene->create_entity();
    Entity* child_entity = scene->create_entity();

    if (!parent_entity || !child_entity) {
        std::cout << "FAILED\n";
        return 1;
    }

    // Set child as child of parent
    if (!child_entity->set_parent(parent_entity)) {
        std::cout << "FAILED\n";
        return 1;
    }

    if (child_entity->get_parent() != parent_entity) {
        std::cout << "FAILED\n";
        return 1;
    }

    const auto& children = parent_entity->get_children();
    if (children.size() != 1 || children[0] != child_entity) {
        std::cout << "FAILED\n";
        return 1;
    }
    std::cout << "PASSED\n";

    // Test 6: Transform operations
    std::cout << "Test 6: Transform operations... ";
    // Set position and scale
    parent_entity->get_transform()->set_position(Vec3(1.0f, 2.0f, 3.0f));
    parent_entity->get_transform()->set_scale(Vec3(2.0f, 2.0f, 2.0f));

    // Verify transform is marked dirty
    if (!parent_entity->get_transform()->is_dirty()) {
        std::cout << "FAILED\n";
        return 1;
    }

    // Update world matrix
    parent_entity->get_transform()->update_world_matrix();

    // Verify transform is no longer dirty
    if (parent_entity->get_transform()->is_dirty()) {
        std::cout << "FAILED\n";
        return 1;
    }
    std::cout << "PASSED\n";

    // Test 6b: Component system
    std::cout << "Test 6b: Component system... ";
    // Test adding a component
    if (entity->has_component<TestComponent>()) {
        std::cout << "FAILED\n";
        return 1;
    }

    TestComponent* test_comp = entity->add_component<TestComponent>();
    if (!test_comp) {
        std::cout << "FAILED\n";
        return 1;
    }

    if (test_comp->value != 42) {
        std::cout << "FAILED\n";
        return 1;
    }

    if (!entity->has_component<TestComponent>()) {
        std::cout << "FAILED\n";
        return 1;
    }

    TestComponent* retrieved = entity->get_component<TestComponent>();
    if (!retrieved || retrieved->value != 42) {
        std::cout << "FAILED\n";
        return 1;
    }

    if (!entity->remove_component<TestComponent>()) {
        std::cout << "FAILED\n";
        return 1;
    }

    if (entity->has_component<TestComponent>()) {
        std::cout << "FAILED\n";
        return 1;
    }
    std::cout << "PASSED\n";

    // Test 6c: Collision system
    std::cout << "Test 6c: Collision system... ";
    // Test AABB collision
    Collision* collision1 = entity->add_component<Collision>();
    Collision* collision2 = parent_entity->add_component<Collision>();

    if (!collision1 || !collision2) {
        std::cout << "FAILED\n";
        return 1;
    }

    // Set AABB collision shapes
    AABB aabb1(Vec3(-1.0f, -1.0f, -1.0f), Vec3(1.0f, 1.0f, 1.0f));
    AABB aabb2(Vec3(0.0f, 0.0f, 0.0f), Vec3(2.0f, 2.0f, 2.0f));

    collision1->set_aabb(aabb1);
    collision2->set_aabb(aabb2);

    // Test collision detection
    CollisionResult result = check_entity_collision(entity, parent_entity);
    if (!result.collided) {
        std::cout << "FAILED\n";
        return 1;
    }
    std::cout << "PASSED\n";

    // Test 6d: Asset system
    std::cout << "Test 6d: Asset system... ";
    AssetManager asset_manager;
    if (!asset_manager.initialize()) {
        std::cout << "FAILED\n";
        return 1;
    }

    // Test asset loading (this will fail since file doesn't exist, but tests the API)
    AssetHandle handle = asset_manager.load_asset("test.txt");
    if (handle.is_valid()) {
        // Asset loaded successfully (unexpected but OK)
    }

    // Test asset type detection
    AssetHandle texture_handle = asset_manager.load_asset("test.png");
    if (texture_handle.is_valid()) {
        Asset* texture_asset = asset_manager.get_asset(texture_handle);
        if (texture_asset && texture_asset->get_type() != AssetType::Texture) {
            std::cout << "FAILED\n";
            return 1;
        }
    }
    std::cout << "PASSED\n";

    // Test 6e: Character controller
    std::cout << "Test 6e: Character controller... ";
    CharacterController* character = entity->add_component<CharacterController>();
    if (!character) {
        std::cout << "FAILED\n";
        return 1;
    }

    // Test character controller properties
    character->set_move_speed(10.0f);
    if (character->get_move_speed() != 10.0f) {
        std::cout << "FAILED\n";
        return 1;
    }

    character->set_jump_velocity(15.0f);
    if (character->get_jump_velocity() != 15.0f) {
        std::cout << "FAILED\n";
        return 1;
    }

    character->set_gravity(-20.0f);
    if (character->get_gravity() != -20.0f) {
        std::cout << "FAILED\n";
        return 1;
    }

    // Test movement
    Vec3 move_dir(1.0f, 0.0f, 0.0f);
    character->move(move_dir, 0.016f);

    // Test facing direction
    character->set_facing_direction(move_dir);
    Vec3 facing = character->get_facing_direction();
    if (std::abs(facing.x - 1.0f) > 0.001f) {
        std::cout << "FAILED\n";
        return 1;
    }
    std::cout << "PASSED\n";

    // Test 6f: World update drives component updates
    std::cout << "Test 6f: World update drives components... ";
    {
        Entity* physics_entity = scene->create_entity("PhysicsEntity");
        CharacterController* pc = physics_entity->add_component<CharacterController>();
        if (!pc) {
            std::cout << "FAILED\n";
            return 1;
        }
        pc->set_velocity(Vec3(2.0f, 0.0f, 0.0f));
        world.update(0.1f);
        float x = physics_entity->get_transform()->get_position().x;
        if (x <= 0.0f) {
            std::cout << "FAILED (position did not integrate, x=" << x << ")\n";
            return 1;
        }
        scene->destroy_entity(physics_entity);
    }
    std::cout << "PASSED\n";

    // Test 6g: Hierarchy world transform propagation
    std::cout << "Test 6g: Hierarchy world transform... ";
    {
        Entity* grandparent = scene->create_entity("Grandparent");
        Entity* mid = scene->create_entity("Mid");
        Entity* leaf = scene->create_entity("Leaf");
        grandparent->set_parent(nullptr);
        mid->set_parent(grandparent);
        leaf->set_parent(mid);

        grandparent->get_transform()->set_position(Vec3(10.0f, 0.0f, 0.0f));
        grandparent->get_transform()->set_scale(Vec3(2.0f, 2.0f, 2.0f));
        mid->get_transform()->set_position(Vec3(1.0f, 0.0f, 0.0f));
        leaf->get_transform()->set_position(Vec3(1.0f, 0.0f, 0.0f));

        // Leaf local (1,0,0) -> mid world: grandparent*(1,0,0)=(12,0,0),
        // mid adds (1,0,0) -> (13,0,0); leaf adds (1,0,0) -> (14,0,0)
        Vec3 leaf_world = leaf->get_transform()->transform_point(Vec3::zero());
        if (std::abs(leaf_world.x - 14.0f) > 0.001f ||
            std::abs(leaf_world.y) > 0.001f ||
            std::abs(leaf_world.z) > 0.001f) {
            std::cout << "FAILED (got " << leaf_world.x << "," << leaf_world.y
                      << "," << leaf_world.z << ")\n";
            return 1;
        }

        // Destroying the middle entity must reparent the leaf to the
        // grandparent instead of leaving a dangling pointer.
        if (!scene->destroy_entity(mid)) {
            std::cout << "FAILED (destroy mid)\n";
            return 1;
        }
        if (leaf->get_parent() != grandparent) {
            std::cout << "FAILED (leaf not reparented)\n";
            return 1;
        }
        scene->destroy_entity(leaf);
        scene->destroy_entity(grandparent);
    }
    std::cout << "PASSED\n";

    // Test 6h: Collision respects entity world transforms
    std::cout << "Test 6h: Collision world transforms... ";
    {
        Entity* near_ent = scene->create_entity("Near");
        Entity* far_ent = scene->create_entity("Far");
        Collision* c_near = near_ent->add_component<Collision>();
        Collision* c_far = far_ent->add_component<Collision>();
        AABB unit(Vec3(-1.0f, -1.0f, -1.0f), Vec3(1.0f, 1.0f, 1.0f));
        c_near->set_aabb(unit);
        c_far->set_aabb(unit);

        // Local bounds are identical, but the far entity is 5 units away in X
        far_ent->get_transform()->set_position(Vec3(5.0f, 0.0f, 0.0f));
        if (check_entity_collision(near_ent, far_ent).collided) {
            std::cout << "FAILED (collided across 5-unit gap)\n";
            return 1;
        }

        // Move it into range: world bounds now overlap
        far_ent->get_transform()->set_position(Vec3(1.5f, 0.0f, 0.0f));
        if (!check_entity_collision(near_ent, far_ent).collided) {
            std::cout << "FAILED (no collision at 1.5 units)\n";
            return 1;
        }
        scene->destroy_entity(near_ent);
        scene->destroy_entity(far_ent);
    }
    std::cout << "PASSED\n";

    // Test 6i: Scene raycast against world-space colliders
    std::cout << "Test 6i: Scene raycast... ";
    {
        // Dedicated scene so leftovers from earlier tests can't interfere
        Scene* ray_scene = world.create_scene("RaycastScene");
        if (!ray_scene) {
            std::cout << "FAILED (create scene)\n";
            return 1;
        }
        Entity* target = ray_scene->create_entity("Target");
        Collision* tc = target->add_component<Collision>();
        tc->set_aabb(AABB(Vec3(-1.0f, -1.0f, -1.0f), Vec3(1.0f, 1.0f, 1.0f)));

        Ray hit_ray(Vec3(0.0f, 0.0f, 5.0f), Vec3(0.0f, 0.0f, -1.0f));
        RaycastResult hit = raycast_scene(ray_scene, hit_ray, 100.0f);
        if (!hit.hit || hit.entity != target || std::abs(hit.distance - 4.0f) > 0.01f) {
            std::cout << "FAILED (hit=" << hit.hit << " dist=" << hit.distance << ")\n";
            return 1;
        }

        Ray miss_ray(Vec3(10.0f, 10.0f, 5.0f), Vec3(0.0f, 0.0f, -1.0f));
        if (raycast_scene(ray_scene, miss_ray, 100.0f).hit) {
            std::cout << "FAILED (unexpected hit)\n";
            return 1;
        }
        world.destroy_scene(ray_scene);
    }
    std::cout << "PASSED\n";

    // Test 6j: Asset deduplication and unload
    std::cout << "Test 6j: Asset deduplication... ";
    {
        const char* tmp_path = "test_asset_dedupe.tmp";
        {
            std::ofstream out(tmp_path, std::ios::binary);
            out << "dedupe-test-payload";
        }
        AssetManager am;
        am.initialize();
        AssetHandle h1 = am.load_asset(tmp_path);
        AssetHandle h2 = am.load_asset("./" + std::string(tmp_path));
        AssetHandle h3 = am.load_model(tmp_path);
        if (!h1.is_valid() || h1 != h2 || h1 != h3) {
            std::cout << "FAILED (handles differ)\n";
            return 1;
        }
        if (am.get_asset_count() != 1) {
            std::cout << "FAILED (count=" << am.get_asset_count() << ")\n";
            return 1;
        }
        // Texture loader on a non-image extension still dedupes to same file
        AssetHandle h4 = am.load_texture(tmp_path, 4, 4, "RGBA8");
        if (h4 != h1) {
            std::cout << "FAILED (texture load not deduped)\n";
            return 1;
        }
        if (!am.unload_asset(h1) || am.get_asset(h1) != nullptr) {
            std::cout << "FAILED (unload)\n";
            return 1;
        }
        // Path is free again: reload yields a fresh handle
        AssetHandle h5 = am.load_asset(tmp_path);
        if (!h5.is_valid() || h5 == h1) {
            std::cout << "FAILED (reload after unload)\n";
            return 1;
        }
        std::remove(tmp_path);
    }
    std::cout << "PASSED\n";

    // Test 6k: set_from_matrix preserves rotation
    std::cout << "Test 6k: set_from_matrix rotation... ";
    {
        Mat4 m = Mat4::translation(Vec3(5.0f, 6.0f, 7.0f)) * Mat4::rotation_y(HALF_PI);
        Transform t;
        t.set_from_matrix(m);
        Vec3 fwd = t.transform_direction(Vec3::forward());
        // rotation_y(+90deg) maps +Z to +X
        if (std::abs(fwd.x - 1.0f) > 0.01f || std::abs(fwd.z) > 0.01f) {
            std::cout << "FAILED (forward=" << fwd.x << "," << fwd.y << "," << fwd.z << ")\n";
            return 1;
        }
        Vec3 pos = t.get_position();
        if (std::abs(pos.x - 5.0f) > 0.001f || std::abs(pos.y - 6.0f) > 0.001f ||
            std::abs(pos.z - 7.0f) > 0.001f) {
            std::cout << "FAILED (position)\n";
            return 1;
        }
    }
    std::cout << "PASSED\n";

    // Test 6l: Stable entity/scene IDs and lookup
    std::cout << "Test 6l: Stable IDs and lookup... ";
    {
        uint64_t scene_id = scene->get_handle().id();
        if (scene_id == 0 || world.find_scene(scene_id) != scene) {
            std::cout << "FAILED (scene lookup)\n";
            return 1;
        }
        Entity* e = scene->create_entity("Named");
        uint64_t eid = e->get_handle().id();
        if (eid == 0 || scene->find_entity(eid) != e) {
            std::cout << "FAILED (entity lookup)\n";
            return 1;
        }
        if (e->get_name() != "Named") {
            std::cout << "FAILED (entity name)\n";
            return 1;
        }
        scene->destroy_entity(e);
        if (scene->find_entity(eid) != nullptr) {
            std::cout << "FAILED (stale entity lookup)\n";
            return 1;
        }
    }
    std::cout << "PASSED\n";

    // Test 6m: Character collision grounding
    std::cout << "Test 6m: Character collision grounding... ";
    {
        Scene* ground_scene = world.create_scene("GroundScene");
        if (!ground_scene) {
            std::cout << "FAILED (create scene)\n";
            return 1;
        }

        // Floor: 4x1x4 box centered at y=-0.5 (top surface at y=0)
        Entity* floor_ent = ground_scene->create_entity("Floor");
        floor_ent->get_transform()->set_position(Vec3(0.0f, -0.5f, 0.0f));
        Collision* floor_col = floor_ent->add_component<Collision>();
        floor_col->set_aabb(AABB(Vec3(-2.0f, -0.5f, -2.0f), Vec3(2.0f, 0.5f, 2.0f)));

        // Character standing on the floor via collision probe
        Entity* walker = ground_scene->create_entity("Walker");
        walker->get_transform()->set_position(Vec3(0.0f, 0.05f, 0.0f));
        CharacterController* wc = walker->add_component<CharacterController>();
        wc->set_collision_grounding(true);
        wc->set_ground_probe_distance(0.5f);
        wc->set_ground_height(-100.0f);  // plane far below: only collider counts

        // Let it settle for a few frames
        for (int i = 0; i < 10; ++i) {
            world.update(0.016f);
        }
        float wy = walker->get_transform()->get_position().y;
        if (!wc->is_grounded() || std::abs(wy) > 0.02f) {
            std::cout << "FAILED (y=" << wy << " grounded=" << wc->is_grounded() << ")\n";
            return 1;
        }

        // Walk off the edge: should fall (no ground within probe)
        walker->get_transform()->set_position(Vec3(10.0f, 0.05f, 0.0f));
        world.update(0.016f);
        if (wc->is_grounded()) {
            std::cout << "FAILED (grounded in mid-air)\n";
            return 1;
        }
        world.destroy_scene(ground_scene);
    }
    std::cout << "PASSED\n";

    // Test 7: World update
    std::cout << "Test 7: World update... ";
    world.update(0.016f);  // 60 FPS
    scene->update(0.016f);
    std::cout << "PASSED\n";

    // Test 8: Entity destruction
    std::cout << "Test 8: Entity destruction... ";
    if (!scene->destroy_entity(child_entity)) {
        std::cout << "FAILED\n";
        return 1;
    }
    std::cout << "PASSED\n";

    // Test 9: Scene destruction
    std::cout << "Test 9: Scene destruction... ";
    if (!world.destroy_scene(scene)) {
        std::cout << "FAILED\n";
        return 1;
    }
    std::cout << "PASSED\n";

    std::cout << "\n=== All Tests Passed ===\n";
    return 0;
}
