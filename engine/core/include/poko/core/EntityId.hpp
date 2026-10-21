#pragma once

#include <cstdint>
#include <functional>
#include <limits>

namespace poko::core {

/**
 * @brief Generational entity ID for detecting stale handles
 *
 * Uses an index into a dense array and a generation counter.
 * When an entity is destroyed and its slot reused, the generation increments,
 * making old handles with the old generation invalid.
 * This prevents use-after-free bugs with entities.
 */
class EntityId {
public:
    using IndexType = std::uint32_t;
    using GenerationType = std::uint32_t;

    /**
     * @brief Construct an invalid EntityId
     */
    constexpr EntityId() noexcept : index_(0), generation_(0) {}

    /**
     * @brief Construct an EntityId with explicit index and generation
     * @param index The index into the entity array
     * @param generation The generation counter
     */
    constexpr EntityId(IndexType index, GenerationType generation) noexcept
        : index_(index), generation_(generation) {}

    /**
     * @brief Get the index component
     * @return The index
     */
    [[nodiscard]] constexpr IndexType index() const noexcept { return index_; }

    /**
     * @brief Get the generation component
     * @return The generation
     */
    [[nodiscard]] constexpr GenerationType generation() const noexcept { return generation_; }

    /**
     * @brief Check if this EntityId is valid (not the default)
     * @return true if valid, false if invalid
     */
    [[nodiscard]] constexpr bool is_valid() const noexcept {
        return index_ != 0 || generation_ != 0;
    }

    /**
     * @brief Check equality with another EntityId
     * @param other The other EntityId
     * @return true if equal
     */
    [[nodiscard]] constexpr bool operator==(const EntityId& other) const noexcept {
        return index_ == other.index_ && generation_ == other.generation_;
    }

    /**
     * @brief Check inequality with another EntityId
     * @param other The other EntityId
     * @return true if not equal
     */
    [[nodiscard]] constexpr bool operator!=(const EntityId& other) const noexcept {
        return !(*this == other);
    }

    /**
     * @brief Less-than comparison for ordered containers
     * @param other The other EntityId
     * @return true if this < other
     */
    [[nodiscard]] constexpr bool operator<(const EntityId& other) const noexcept {
        if (index_ != other.index_) {
            return index_ < other.index_;
        }
        return generation_ < other.generation_;
    }

    /**
     * @brief Get a hash value for use in unordered containers
     * @return Hash value
     */
    [[nodiscard]] std::size_t hash() const noexcept {
        return static_cast<std::size_t>(index_) ^ 
               (static_cast<std::size_t>(generation_) << 16);
    }

    /**
     * @brief Create an invalid EntityId (alias for default constructor)
     * @return Invalid EntityId
     */
    [[nodiscard]] static constexpr EntityId invalid() noexcept {
        return EntityId{};
    }

private:
    IndexType index_;
    GenerationType generation_;
};

/**
 * @brief Hash function for EntityId in unordered containers
 */
struct EntityIdHash {
    [[nodiscard]] std::size_t operator()(const EntityId& id) const noexcept {
        return id.hash();
    }
};

} // namespace poko::core

// Enable EntityId as key in std::unordered_map
namespace std {

template<>
struct hash<poko::core::EntityId> {
    std::size_t operator()(const poko::core::EntityId& id) const noexcept {
        return id.hash();
    }
};

} // namespace std
