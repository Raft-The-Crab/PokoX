/**
 * @file world.cpp
 * @brief Poko world module - scene graph and ECS storage.
 *
 * This module provides the core world representation for the PokoX engine.
 * It manages scenes, entities, hierarchy, and transform propagation.
 *
 * Production Status: M1 World Foundation
 */

#include "poko/world/world.h"
#include "poko/world/scene.h"
#include "poko/world/entity.h"
#include "poko/world/transform.h"

#include <memory>
#include <unordered_map>
#include <vector>
#include <algorithm>

namespace poko::world {

// -----------------------------------------------------------------------------
// World::Impl - Pimpl pattern for World
// -----------------------------------------------------------------------------

struct World::Impl {
    WorldConfig config;
    std::vector<std::unique_ptr<Scene>> scenes;
    std::unordered_map<uint64_t, Scene*> scene_lookup;
    uint64_t next_scene_id = 1;
    uint64_t next_world_id = 0;
    bool initialized = false;

    Impl() = default;
    ~Impl() = default;
};

// -----------------------------------------------------------------------------
// World implementation
// -----------------------------------------------------------------------------

World::World() : impl_(new Impl()) {}

World::~World() {
    delete impl_;
}

bool World::initialize(const WorldConfig& config) {
    if (impl_->initialized) {
        return true;  // Already initialized
    }

    impl_->config = config;
    impl_->initialized = true;

    // Initialize world ID (could be more sophisticated with UUID)
    impl_->next_world_id = 1;

    return true;
}

bool World::is_valid() const {
    return impl_ && impl_->initialized;
}

Scene* World::create_scene(const std::string& name) {
    if (!impl_->initialized) {
        return nullptr;
    }

    if (impl_->scenes.size() >= impl_->config.max_scenes) {
        return nullptr;  // Max scenes reached
    }

    uint64_t scene_id = impl_->next_scene_id++;

    auto scene = std::make_unique<Scene>();
    Scene* scene_ptr = scene.get();

    if (!scene_ptr->initialize(name, this, scene_id)) {
        return nullptr;
    }

    scene_ptr->set_max_entities(impl_->config.max_entities);
    impl_->scene_lookup[scene_id] = scene_ptr;
    impl_->scenes.push_back(std::move(scene));

    return scene_ptr;
}

Scene* World::find_scene(uint64_t id) const {
    auto it = impl_->scene_lookup.find(id);
    return (it != impl_->scene_lookup.end()) ? it->second : nullptr;
}

bool World::destroy_scene(Scene* scene) {
    if (!scene || !scene->is_valid()) {
        return false;
    }

    // Verify the scene belongs to this world via its stable ID
    auto lookup_it = impl_->scene_lookup.find(scene->get_handle().id());
    if (lookup_it == impl_->scene_lookup.end() || lookup_it->second != scene) {
        return false;
    }

    // Find and remove from scenes vector
    auto it = std::find_if(impl_->scenes.begin(), impl_->scenes.end(),
        [scene](const std::unique_ptr<Scene>& s) { return s.get() == scene; });

    if (it == impl_->scenes.end()) {
        return false;
    }

    impl_->scene_lookup.erase(lookup_it);
    impl_->scenes.erase(it);
    return true;
}

const std::vector<Scene*>& World::get_scenes() const {
    // Return actual scenes - need to convert unique_ptr to raw pointers
    static thread_local std::vector<Scene*> scene_ptrs;
    scene_ptrs.clear();
    scene_ptrs.reserve(impl_->scenes.size());

    for (const auto& scene : impl_->scenes) {
        scene_ptrs.push_back(scene.get());
    }

    return scene_ptrs;
}

uint32_t World::get_entity_count() const {
    uint32_t count = 0;
    for (const auto& scene : impl_->scenes) {
        count += scene->get_entity_count();
    }
    return count;
}

void World::update(float delta_time) {
    if (!impl_->initialized) {
        return;
    }

    // Scenes update their entities' components and transform world matrices.
    // Index-based iteration guards against scenes created/destroyed during
    // callbacks.
    for (size_t i = 0; i < impl_->scenes.size(); ++i) {
        impl_->scenes[i]->update(delta_time);
    }
}

WorldHandle World::get_handle() const {
    return WorldHandle(impl_->next_world_id);
}

} // namespace poko::world
