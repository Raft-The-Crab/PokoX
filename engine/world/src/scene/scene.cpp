/**
 * @file scene.cpp
 * @brief Poko scene - container for entities and components.
 *
 * A scene represents a logical grouping of entities, typically corresponding
 * to a level, menu, or game state. Scenes manage their own entity sets and
 * provide efficient batch operations.
 *
 * Production Status: M1 World Foundation
 */

#include "poko/world/scene.h"
#include "poko/world/entity.h"
#include "poko/world/world.h"

#include <memory>
#include <unordered_map>
#include <vector>
#include <algorithm>

namespace poko::world {

// -----------------------------------------------------------------------------
// Scene::Impl - Pimpl pattern for Scene
// -----------------------------------------------------------------------------

struct Scene::Impl {
    std::string name;
    World* world = nullptr;
    std::vector<std::unique_ptr<Entity>> entities;
    std::unordered_map<uint64_t, Entity*> entity_lookup;
    uint64_t next_entity_id = 1;
    uint64_t scene_id = 0;
    uint32_t max_entities = 0;  // 0 = unlimited
    bool initialized = false;

    Impl() = default;
    ~Impl() = default;
};

// -----------------------------------------------------------------------------
// Scene implementation
// -----------------------------------------------------------------------------

Scene::Scene() : impl_(new Impl()) {}

Scene::~Scene() {
    delete impl_;
}

bool Scene::initialize(const std::string& name, World* world, uint64_t id) {
    if (impl_->initialized) {
        return true;  // Already initialized
    }

    impl_->name = name;
    impl_->world = world;
    impl_->initialized = true;

    // Use the caller-provided stable ID when available; otherwise fall back
    // to the instance address (still unique, but not stable across realloc).
    impl_->scene_id = (id != 0) ? id : reinterpret_cast<uint64_t>(this);

    return true;
}

bool Scene::is_valid() const {
    return impl_ && impl_->initialized;
}

const std::string& Scene::get_name() const {
    return impl_->name;
}

SceneHandle Scene::get_handle() const {
    return SceneHandle(impl_->scene_id);
}

Entity* Scene::create_entity(const std::string& name) {
    if (!impl_->initialized) {
        return nullptr;
    }

    if (impl_->max_entities != 0 &&
        impl_->entities.size() >= impl_->max_entities) {
        return nullptr;  // Entity limit reached
    }

    uint64_t entity_id = impl_->next_entity_id++;

    auto entity = std::make_unique<Entity>();
    Entity* entity_ptr = entity.get();

    if (!entity_ptr->initialize(name, this, entity_id)) {
        return nullptr;
    }

    impl_->entity_lookup[entity_id] = entity_ptr;
    impl_->entities.push_back(std::move(entity));

    return entity_ptr;
}

Entity* Scene::find_entity(uint64_t id) const {
    auto it = impl_->entity_lookup.find(id);
    return (it != impl_->entity_lookup.end()) ? it->second : nullptr;
}

bool Scene::destroy_entity(Entity* entity) {
    if (!entity || !entity->is_valid() || entity->get_scene() != this) {
        return false;
    }

    // Find and remove from entities vector
    auto it = std::find_if(impl_->entities.begin(), impl_->entities.end(),
        [entity](const std::unique_ptr<Entity>& e) { return e.get() == entity; });

    if (it == impl_->entities.end()) {
        return false;
    }

    // Remove from lookup using the entity's stable ID
    impl_->entity_lookup.erase(entity->get_handle().id());

    // Erasing the unique_ptr destroys the entity; its destructor reparents
    // children and detaches from its parent, so no dangling pointers remain.
    impl_->entities.erase(it);
    return true;
}

const std::vector<Entity*>& Scene::get_entities() const {
    // Return actual entities - need to convert unique_ptr to raw pointers
    static thread_local std::vector<Entity*> entity_ptrs;
    entity_ptrs.clear();
    entity_ptrs.reserve(impl_->entities.size());

    for (const auto& entity : impl_->entities) {
        entity_ptrs.push_back(entity.get());
    }

    return entity_ptrs;
}

uint32_t Scene::get_entity_count() const {
    return static_cast<uint32_t>(impl_->entities.size());
}

void Scene::set_max_entities(uint32_t max_entities) {
    impl_->max_entities = max_entities;
}

void Scene::update(float delta_time) {
    if (!impl_->initialized) {
        return;
    }

    // Index-based iteration: entities may be created or destroyed by
    // component updates during the loop, which would invalidate iterators.
    for (size_t i = 0; i < impl_->entities.size(); ++i) {
        Entity* entity = impl_->entities[i].get();
        entity->update_components(delta_time);
        if (Transform* transform = entity->get_transform()) {
            transform->update_world_matrix();
        }
    }
}

} // namespace poko::world
