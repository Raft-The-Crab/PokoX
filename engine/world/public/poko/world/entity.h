/**
 * @file entity.h
 * @brief Poko entity - basic unit of game state.
 *
 * An entity represents a game object in the world. It can have components
 * (data) and participate in the scene hierarchy. Entities use stable handles
 * for safe referencing across systems.
 *
 * Production Status: M1 World Foundation
 */

#ifndef POKO_WORLD_ENTITY_H
#define POKO_WORLD_ENTITY_H

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include "poko/world/component.h"
#include "poko/world/transform.h"

namespace poko::world {

// Forward declarations
class Scene;
class Transform;

/**
 * @brief Opaque handle to an entity.
 */
class EntityHandle {
public:
    EntityHandle() : id_(0) {}
    explicit EntityHandle(uint64_t id) : id_(id) {}

    bool is_valid() const { return id_ != 0; }
    uint64_t id() const { return id_; }

    bool operator==(const EntityHandle& other) const { return id_ == other.id_; }
    bool operator!=(const EntityHandle& other) const { return id_ != other.id_; }

private:
    uint64_t id_;
};

/**
 * @brief Entity interface - basic game object.
 */
class Entity {
public:
    Entity();
    ~Entity();

    // Non-copyable, non-movable
    Entity(const Entity&) = delete;
    Entity& operator=(const Entity&) = delete;
    Entity(Entity&&) = delete;
    Entity& operator=(Entity&&) = delete;

    /**
     * @brief Initializes the entity.
     *
     * @param name Entity name (optional).
     * @param scene Parent scene.
     * @param id Stable entity ID (0 generates an ID from the instance address).
     * @return true on success, false on failure.
     */
    bool initialize(const std::string& name, Scene* scene, uint64_t id = 0);

    /**
     * @brief Returns true if the entity is valid.
     */
    bool is_valid() const;

    /**
     * @brief Gets the entity name.
     */
    const std::string& get_name() const;

    /**
     * @brief Sets the entity name.
     *
     * @param name New name.
     */
    void set_name(const std::string& name);

    /**
     * @brief Gets the entity handle.
     */
    EntityHandle get_handle() const;

    /**
     * @brief Gets the parent scene.
     */
    Scene* get_scene() const;

    /**
     * @brief Gets the entity's transform.
     */
    Transform* get_transform() const;

    /**
     * @brief Sets the entity's parent (for hierarchy).
     *
     * @param parent New parent entity (nullptr for root).
     * @return true on success, false on failure.
     */
    bool set_parent(Entity* parent);

    /**
     * @brief Gets the entity's parent.
     */
    Entity* get_parent() const;

    /**
     * @brief Gets the entity's children.
     */
    const std::vector<Entity*>& get_children() const;

    /**
     * @brief Checks if the entity has a specific component.
     *
     * @tparam T Component type (must inherit from Component).
     * @return true if component exists, false otherwise.
     */
    template<typename T>
    bool has_component() const;

    /**
     * @brief Gets a component by type.
     *
     * @tparam T Component type.
     * @return Pointer to component or nullptr if not found.
     */
    template<typename T>
    T* get_component() const;

    /**
     * @brief Adds a component to the entity.
     *
     * @tparam T Component type (must inherit from Component).
     * @return Pointer to the new component or nullptr on failure.
     */
    template<typename T>
    T* add_component();

    /**
     * @brief Removes a component from the entity.
     *
     * @tparam T Component type.
     * @return true on success, false on failure.
     */
    template<typename T>
    bool remove_component();

    /**
     * @brief Gets all components as handles.
     */
    std::vector<ComponentHandle> get_all_components() const;

    /**
     * @brief Updates all components.
     *
     * @param delta_time Time since last update (seconds).
     */
    void update_components(float delta_time);

private:
    struct Impl {
        std::string name;
        Scene* scene = nullptr;
        Entity* parent = nullptr;
        std::vector<Entity*> children;
        std::unique_ptr<Transform> transform;
        ComponentStorage component_storage;
        uint64_t entity_id = 0;
        bool initialized = false;

        Impl() = default;
        ~Impl() = default;
    };
    Impl* impl_;
};

// Template implementations (must be in header for template instantiation)
template<typename T>
bool Entity::has_component() const {
    return impl_->component_storage.has_component<T>();
}

template<typename T>
T* Entity::get_component() const {
    return impl_->component_storage.get_component<T>();
}

template<typename T>
T* Entity::add_component() {
    auto component = std::make_unique<T>();
    T* component_ptr = component.get();

    if (!impl_->component_storage.add_component(std::move(component))) {
        return nullptr;  // Component already exists or failed to add
    }

    // Call lifecycle hook
    component_ptr->on_add(this);
    return component_ptr;
}

template<typename T>
bool Entity::remove_component() {
    T* component = impl_->component_storage.get_component<T>();
    if (!component) {
        return false;
    }

    // Call lifecycle hook
    component->on_remove(this);

    return impl_->component_storage.remove_component<T>();
}

} // namespace poko::world

#endif // POKO_WORLD_ENTITY_H
