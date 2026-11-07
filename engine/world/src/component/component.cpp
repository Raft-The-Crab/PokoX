/**
 * @file component.cpp
 * @brief Component system implementation.
 *
 * Production Status: M1 World Foundation (Production)
 */

#include "poko/world/component.h"
#include <utility>
#include <vector>

namespace poko::world {

ComponentStorage::ComponentStorage() = default;
ComponentStorage::~ComponentStorage() = default;

std::vector<ComponentHandle> ComponentStorage::get_all_components() const {
    std::vector<ComponentHandle> handles;
    handles.reserve(components_.size());

    for (const auto& pair : components_) {
        handles.emplace_back(pair.second.get(), pair.first);
    }

    return handles;
}

void ComponentStorage::update(float delta_time) {
    // Snapshot (type, component) pairs so that components added or removed
    // during the update cannot invalidate iteration. A component removed
    // mid-update is skipped; a re-added component of the same type is a
    // different instance and is also skipped.
    std::vector<std::pair<std::type_index, Component*>> snapshot;
    snapshot.reserve(components_.size());
    for (const auto& pair : components_) {
        snapshot.emplace_back(pair.first, pair.second.get());
    }

    for (const auto& entry : snapshot) {
        auto it = components_.find(entry.first);
        if (it != components_.end() && it->second.get() == entry.second) {
            entry.second->on_update(delta_time);
        }
    }
}

} // namespace poko::world
