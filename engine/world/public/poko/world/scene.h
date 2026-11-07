/**
 * @file scene.h
 * @brief Poko scene - container for entities and components.
 *
 * A scene represents a logical grouping of entities, typically corresponding
 * to a level, menu, or game state. Scenes manage their own entity sets and
 * provide efficient batch operations.
 *
 * Production Status: M1 World Foundation
 */

#ifndef POKO_WORLD_SCENE_H
#define POKO_WORLD_SCENE_H

#include <cstdint>
#include <string>
#include <vector>

namespace poko::world {

// Forward declarations
class Entity;
class World;

/**
 * @brief Opaque handle to a scene.
 */
class SceneHandle {
public:
    SceneHandle() : id_(0) {}
    explicit SceneHandle(uint64_t id) : id_(id) {}

    bool is_valid() const { return id_ != 0; }
    uint64_t id() const { return id_; }

    bool operator==(const SceneHandle& other) const { return id_ == other.id_; }
    bool operator!=(const SceneHandle& other) const { return id_ != other.id_; }

private:
    uint64_t id_;
};

/**
 * @brief Scene interface - container for entities.
 */
class Scene {
public:
    Scene();
    ~Scene();

    // Non-copyable, non-movable
    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;
    Scene(Scene&&) = delete;
    Scene& operator=(Scene&&) = delete;

    /**
     * @brief Initializes the scene.
     *
     * @param name Scene name.
     * @param world Parent world.
     * @param id Stable scene ID (0 generates an ID from the instance address).
     * @return true on success, false on failure.
     */
    bool initialize(const std::string& name, World* world, uint64_t id = 0);

    /**
     * @brief Returns true if the scene is valid.
     */
    bool is_valid() const;

    /**
     * @brief Gets the scene name.
     */
    const std::string& get_name() const;

    /**
     * @brief Gets the scene handle.
     */
    SceneHandle get_handle() const;

    /**
     * @brief Creates a new entity in this scene.
     *
     * @param name Entity name (optional).
     * @return Entity pointer or nullptr on failure.
     */
    Entity* create_entity(const std::string& name = "");

    /**
     * @brief Finds an entity by its stable ID.
     *
     * @param id Entity ID (from EntityHandle::id()).
     * @return Entity pointer or nullptr if not found.
     */
    Entity* find_entity(uint64_t id) const;

    /**
     * @brief Destroys an entity.
     *
     * @param entity Entity to destroy.
     * @return true on success, false on failure.
     */
    bool destroy_entity(Entity* entity);

    /**
     * @brief Gets all entities in this scene.
     */
    const std::vector<Entity*>& get_entities() const;

    /**
     * @brief Gets the number of entities in this scene.
     */
    uint32_t get_entity_count() const;

    /**
     * @brief Sets the maximum number of entities allowed in this scene.
     *
     * @param max_entities Maximum entity count (0 = unlimited).
     */
    void set_max_entities(uint32_t max_entities);

    /**
     * @brief Updates the scene (called once per frame).
     *
     * @param delta_time Time since last update (seconds).
     */
    void update(float delta_time);

private:
    struct Impl;
    Impl* impl_;
};

} // namespace poko::world

#endif // POKO_WORLD_SCENE_H
