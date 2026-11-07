/**
 * @file entity.cpp
 * @brief Poko entity - basic unit of game state.
 *
 * An entity represents a game object in the world. It can have components
 * (data) and participate in the scene hierarchy. Entities use stable handles
 * for safe referencing across systems.
 *
 * Production Status: M1 World Foundation
 */

#include "poko/world/entity.h"
#include "poko/world/scene.h"
#include "poko/world/transform.h"

#include <memory>
#include <vector>
#include <unordered_map>
#include <typeindex>
#include <algorithm>

namespace poko::world {

// Entity::Impl is now defined in the header file for template instantiation

// -----------------------------------------------------------------------------
// Entity implementation
// -----------------------------------------------------------------------------

Entity::Entity() : impl_(new Impl()) {}

Entity::~Entity() {
    // Reparent children before detaching: they must never keep a pointer to
    // this (soon to be freed) entity. Children adopt this entity's parent so
    // the hierarchy stays intact.
    Entity* new_parent = impl_->parent;
    for (Entity* child : impl_->children) {
        if (!child) {
            continue;
        }
        child->impl_->parent = new_parent;
        if (new_parent) {
            new_parent->impl_->children.push_back(child);
        }
        if (child->impl_->transform && impl_->transform) {
            child->impl_->transform->set_parent(
                new_parent ? new_parent->impl_->transform.get() : nullptr);
        }
    }
    impl_->children.clear();

    // Detach from parent so it no longer references this entity.
    if (impl_->parent) {
        auto& siblings = impl_->parent->impl_->children;
        siblings.erase(std::remove(siblings.begin(), siblings.end(), this), siblings.end());
        impl_->parent = nullptr;
    }

    delete impl_;
}

bool Entity::initialize(const std::string& name, Scene* scene, uint64_t id) {
    if (impl_->initialized) {
        return true;  // Already initialized
    }

    impl_->name = name;
    impl_->scene = scene;
    impl_->transform = std::make_unique<Transform>();
    impl_->transform->initialize();
    impl_->initialized = true;

    // Use the caller-provided stable ID when available; otherwise fall back
    // to the instance address (still unique, but not stable across realloc).
    impl_->entity_id = (id != 0) ? id : reinterpret_cast<uint64_t>(this);

    return true;
}

bool Entity::is_valid() const {
    return impl_ && impl_->initialized;
}

const std::string& Entity::get_name() const {
    return impl_->name;
}

void Entity::set_name(const std::string& name) {
    impl_->name = name;
}

EntityHandle Entity::get_handle() const {
    return EntityHandle(impl_->entity_id);
}

Scene* Entity::get_scene() const {
    return impl_->scene;
}

Transform* Entity::get_transform() const {
    return impl_->transform.get();
}

bool Entity::set_parent(Entity* parent) {
    // Prevent circular references
    if (parent == this) {
        return false;
    }

    // Check if parent is a descendant of this entity
    Entity* current = parent;
    while (current) {
        if (current == this) {
            return false;  // Would create circular reference
        }
        current = current->impl_->parent;
    }

    // Remove from old parent's children
    if (impl_->parent) {
        auto& siblings = impl_->parent->impl_->children;
        auto it = std::find(siblings.begin(), siblings.end(), this);
        if (it != siblings.end()) {
            siblings.erase(it);
        }
    }

    // Set new parent
    impl_->parent = parent;

    // Add to new parent's children
    if (parent) {
        parent->impl_->children.push_back(this);

        // Update transform parent
        if (impl_->transform && parent->impl_->transform) {
            impl_->transform->set_parent(parent->impl_->transform.get());
        }
    } else {
        // Remove parent from transform
        if (impl_->transform) {
            impl_->transform->set_parent(nullptr);
        }
    }

    return true;
}

Entity* Entity::get_parent() const {
    return impl_->parent;
}

const std::vector<Entity*>& Entity::get_children() const {
    return impl_->children;
}

// Template implementations are now in the header file for proper instantiation
// This file only contains non-template member function implementations

std::vector<ComponentHandle> Entity::get_all_components() const {
    return impl_->component_storage.get_all_components();
}

void Entity::update_components(float delta_time) {
    impl_->component_storage.update(delta_time);
}

} // namespace poko::world
