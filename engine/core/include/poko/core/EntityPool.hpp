#pragma once

#include "poko/core/EntityId.hpp"
#include <vector>
#include <cstdint>
#include <optional>

namespace poko::core {

/**
 * @brief Entity pool for managing EntityId allocation and reuse
 *
 * Manages a pool of entities with generational handles for stale-handle detection.
 * When an entity is destroyed, its slot is marked as free and reused with an incremented generation.
 */
class EntityPool {
public:
    /**
     * @brief Maximum number of entities in the pool
     */
    static constexpr std::uint32_t MAX_ENTITIES = 65536;

    /**
     * @brief Entity slot status
     */
    struct Slot {
        std::uint32_t generation;
        bool alive;
    };

    /**
     * @brief Construct an entity pool
     */
    EntityPool();

    /**
     * @brief Allocate a new entity
     * @return The allocated EntityId, or invalid if pool is full
     */
    [[nodiscard]] EntityId allocate();

    /**
     * @brief Deallocate an entity
     * @param id The EntityId to deallocate
     * @return true if deallocated, false if ID was invalid or already dead
     */
    bool deallocate(EntityId id);

    /**
     * @brief Check if an entity is alive
     * @param id The EntityId to check
     * @return true if alive, false otherwise
     */
    [[nodiscard]] bool is_alive(EntityId id) const;

    /**
     * @brief Get the number of alive entities
     * @return Count of alive entities
     */
    [[nodiscard]] std::uint32_t alive_count() const noexcept { return alive_count_; }

    /**
     * @brief Get the number of free slots
     * @return Count of free slots
     */
    [[nodiscard]] std::uint32_t free_count() const noexcept { return free_indices_.size(); }

    /**
     * @brief Check if the pool is full
     * @return true if full, false otherwise
     */
    [[nodiscard]] bool is_full() const noexcept { return free_indices_.empty(); }

private:
    std::vector<Slot> slots_;
    std::vector<std::uint32_t> free_indices_;
    std::uint32_t alive_count_;
};

} // namespace poko::core
