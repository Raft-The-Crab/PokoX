/**
 * @file world.h
 * @brief Poko world module - scene graph and ECS storage.
 *
 * This module provides the core world representation for the PokoX engine.
 * It manages scenes, entities, hierarchy, and transform propagation.
 *
 * Design principles:
 * - ECS storage for efficient component access
 * - Stable object handles for entity references
 * - Hierarchy support for parent-child relationships
 * - Transform propagation for efficient world matrix computation
 * - Serialization boundaries for save/load functionality
 *
 * Dependencies:
 * - poko_core: Engine infrastructure
 * - poko_math: Vector, matrix, quaternion math
 *
 * Production Status: M1 World Foundation
 */

#ifndef POKO_WORLD_WORLD_H
#define POKO_WORLD_WORLD_H

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace poko::world {

// Forward declarations
class World;
class Scene;
class Entity;

/**
 * @brief Opaque handle to a world instance.
 */
class WorldHandle {
public:
    WorldHandle() : id_(0) {}
    explicit WorldHandle(uint64_t id) : id_(id) {}

    bool is_valid() const { return id_ != 0; }
    uint64_t id() const { return id_; }

    bool operator==(const WorldHandle& other) const { return id_ == other.id_; }
    bool operator!=(const WorldHandle& other) const { return id_ != other.id_; }

private:
    uint64_t id_;
};

/**
 * @brief World configuration.
 */
struct WorldConfig {
    uint32_t max_entities = 65536;      // Maximum entities per world
    uint32_t max_scenes = 256;          // Maximum scenes per world
    bool enable_hierarchy = true;       // Enable parent-child relationships
    bool enable_serialization = true;   // Enable serialization support
};

/**
 * @brief World interface - manages scenes and entities.
 *
 * The World is the top-level container for all game state. It owns scenes,
 * manages entity lifecycle, and provides access to the ECS storage.
 */
class World {
public:
    World();
    ~World();

    // Non-copyable, non-movable
    World(const World&) = delete;
    World& operator=(const World&) = delete;
    World(World&&) = delete;
    World& operator=(World&&) = delete;

    /**
     * @brief Initializes the world with configuration.
     *
     * @param config World configuration.
     * @return true on success, false on failure.
     */
    bool initialize(const WorldConfig& config);

    /**
     * @brief Returns true if the world is valid.
     */
    bool is_valid() const;

    /**
     * @brief Creates a new scene.
     *
     * @param name Scene name (optional).
     * @return Scene handle or invalid handle on failure.
     */
    Scene* create_scene(const std::string& name = "");

    /**
     * @brief Destroys a scene.
     *
     * @param scene Scene to destroy.
     * @return true on success, false on failure.
     */
    bool destroy_scene(Scene* scene);

    /**
     * @brief Finds a scene by its stable ID.
     *
     * @param id Scene ID (from SceneHandle::id()).
     * @return Scene pointer or nullptr if not found.
     */
    Scene* find_scene(uint64_t id) const;

    /**
     * @brief Gets all scenes in the world.
     */
    const std::vector<Scene*>& get_scenes() const;

    /**
     * @brief Gets the number of entities in the world.
     */
    uint32_t get_entity_count() const;

    /**
     * @brief Updates the world (called once per frame).
     *
     * @param delta_time Time since last update (seconds).
     */
    void update(float delta_time);

    /**
     * @brief Gets the world handle.
     */
    WorldHandle get_handle() const;

private:
    struct Impl;
    Impl* impl_;
};

} // namespace poko::world

#endif // POKO_WORLD_WORLD_H
