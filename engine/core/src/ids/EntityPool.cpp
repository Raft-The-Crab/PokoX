#include "poko/core/EntityPool.hpp"

namespace poko::core {

EntityPool::EntityPool() : alive_count_(0) {
    slots_.reserve(MAX_ENTITIES);
    free_indices_.reserve(MAX_ENTITIES);
    
    // Pre-populate free indices
    for (std::uint32_t i = 0; i < MAX_ENTITIES; ++i) {
        free_indices_.push_back(i);
        slots_.push_back({0, false});
    }
}

EntityId EntityPool::allocate() {
    if (free_indices_.empty()) {
        return EntityId::invalid();
    }
    
    std::uint32_t index = free_indices_.back();
    free_indices_.pop_back();
    
    Slot& slot = slots_[index];
    slot.alive = true;
    slot.generation++;
    
    alive_count_++;
    
    return EntityId(index, slot.generation);
}

bool EntityPool::deallocate(EntityId id) {
    if (!id.is_valid()) {
        return false;
    }
    
    std::uint32_t index = id.index();
    
    if (index >= slots_.size()) {
        return false;
    }
    
    Slot& slot = slots_[index];
    
    if (!slot.alive) {
        return false;
    }
    
    if (slot.generation != id.generation()) {
        return false; // Stale handle
    }
    
    slot.alive = false;
    free_indices_.push_back(index);
    alive_count_--;
    
    return true;
}

bool EntityPool::is_alive(EntityId id) const {
    if (!id.is_valid()) {
        return false;
    }
    
    std::uint32_t index = id.index();
    
    if (index >= slots_.size()) {
        return false;
    }
    
    const Slot& slot = slots_[index];
    
    return slot.alive && slot.generation == id.generation();
}

} // namespace poko::core
