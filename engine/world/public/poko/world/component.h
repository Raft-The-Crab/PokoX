/**
 * @file component.h
 * @brief Type-safe component system for PokoX ECS.
 *
 * Provides a production-ready component storage system with type safety,
 * lifecycle management, and efficient queries.
 *
 * Production Status: M1 World Foundation (Production)
 */

#ifndef POKO_WORLD_COMPONENT_H
#define POKO_WORLD_COMPONENT_H

#include <cstdint>
#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace poko::world {

// Forward declarations
class Entity;
class World;

/**
 * @brief Component interface - base class for all components.
 *
 * All components must inherit from this interface to be stored in the ECS.
 */
class Component {
public:
    Component() = default;
    virtual ~Component() = default;

    // Non-copyable, non-movable
    Component(const Component&) = delete;
    Component& operator=(const Component&) = delete;
    Component(Component&&) = delete;
    Component& operator=(Component&&) = delete;

    /**
     * @brief Called when component is added to entity.
     */
    virtual void on_add(Entity*) {}

    /**
     * @brief Called when component is removed from entity.
     */
    virtual void on_remove(Entity*) {}

    /**
     * @brief Called during world update.
     *
     * @param delta_time Time since last update (seconds).
     */
    virtual void on_update(float) {}
};

/**
 * @brief Component handle - safe reference to a component.
 */
class ComponentHandle {
public:
    ComponentHandle() : component_(nullptr), type_index_(std::type_index(typeid(void))) {}
    ComponentHandle(void* component, std::type_index type) 
        : component_(component), type_index_(type) {}

    bool is_valid() const { return component_ != nullptr; }

    template<typename T>
    T* as() const {
        if (!is_valid() || type_index_ != std::type_index(typeid(T))) {
            return nullptr;
        }
        return static_cast<T*>(component_);
    }

    std::type_index get_type() const { return type_index_; }

private:
    void* component_;
    std::type_index type_index_;
};

/**
 * @brief Component storage - manages components for an entity.
 */
class ComponentStorage {
public:
    ComponentStorage();
    ~ComponentStorage();

    // Non-copyable, non-movable
    ComponentStorage(const ComponentStorage&) = delete;
    ComponentStorage& operator=(const ComponentStorage&) = delete;
    ComponentStorage(ComponentStorage&&) = delete;
    ComponentStorage& operator=(ComponentStorage&&) = delete;

    /**
     * @brief Adds a component to storage.
     *
     * @tparam T Component type (must inherit from Component).
     * @param component Component instance (takes ownership).
     * @return true on success, false on failure.
     */
    template<typename T>
    bool add_component(std::unique_ptr<T> component);

    /**
     * @brief Gets a component by type.
     *
     * @tparam T Component type.
     * @return Pointer to component or nullptr if not found.
     */
    template<typename T>
    T* get_component() const;

    /**
     * @brief Removes a component by type.
     *
     * @tparam T Component type.
     * @return true on success, false on failure.
     */
    template<typename T>
    bool remove_component();

    /**
     * @brief Checks if component exists.
     *
     * @tparam T Component type.
     * @return true if component exists, false otherwise.
     */
    template<typename T>
    bool has_component() const;

    /**
     * @brief Gets all components as handles.
     */
    std::vector<ComponentHandle> get_all_components() const;

    /**
     * @brief Calls on_update on all components.
     *
     * @param delta_time Time since last update (seconds).
     */
    void update(float delta_time);

private:
    std::unordered_map<std::type_index, std::unique_ptr<Component>> components_;
};

// Template implementations
template<typename T>
bool ComponentStorage::add_component(std::unique_ptr<T> component) {
    if (!component) {
        return false;
    }

    std::type_index type = std::type_index(typeid(T));
    if (components_.find(type) != components_.end()) {
        return false;  // Component already exists
    }

    components_[type] = std::move(component);
    return true;
}

template<typename T>
T* ComponentStorage::get_component() const {
    std::type_index type = std::type_index(typeid(T));
    auto it = components_.find(type);
    if (it == components_.end()) {
        return nullptr;
    }
    return static_cast<T*>(it->second.get());
}

template<typename T>
bool ComponentStorage::remove_component() {
    std::type_index type = std::type_index(typeid(T));
    auto it = components_.find(type);
    if (it == components_.end()) {
        return false;
    }
    components_.erase(it);
    return true;
}

template<typename T>
bool ComponentStorage::has_component() const {
    std::type_index type = std::type_index(typeid(T));
    return components_.find(type) != components_.end();
}

} // namespace poko::world

#endif // POKO_WORLD_COMPONENT_H
