#pragma once

#include <cstdint>
#include <functional>
#include <limits>

namespace poko::core {

/**
 * @brief Base template for stable resource IDs
 *
 * A type-safe ID wrapper for identifying resources, assets, etc.
 * Prevents mixing different ID types through type safety.
 * 
 * NON-PRODUCTION: Enhanced ID types beyond the basic EntityId.
 * Production uses only EntityId for entities.
 * 
 * @tparam Tag Type tag to prevent implicit conversion between different ID types
 * @tparam UnderlyingType The underlying integer type
 */
template<typename Tag, typename UnderlyingType = std::uint32_t>
class TypedId {
public:
    using ValueType = UnderlyingType;

    /**
     * @brief Construct an invalid ID (0)
     */
    constexpr TypedId() noexcept : value_(0) {}

    /**
     * @brief Construct an ID with explicit value
     * @param value The ID value
     */
    constexpr explicit TypedId(ValueType value) noexcept : value_(value) {}

    /**
     * @brief Get the underlying value
     * @return The ID value
     */
    [[nodiscard]] constexpr ValueType value() const noexcept { return value_; }

    /**
     * @brief Check if this ID is valid (non-zero)
     * @return true if valid, false otherwise
     */
    [[nodiscard]] constexpr bool is_valid() const noexcept { return value_ != 0; }

    /**
     * @brief Check equality with another ID
     * @param other The other ID
     * @return true if equal
     */
    [[nodiscard]] constexpr bool operator==(const TypedId& other) const noexcept {
        return value_ == other.value_;
    }

    /**
     * @brief Check inequality with another ID
     * @param other The other ID
     * @return true if not equal
     */
    [[nodiscard]] constexpr bool operator!=(const TypedId& other) const noexcept {
        return !(*this == other);
    }

    /**
     * @brief Less-than comparison for ordered containers
     * @param other The other ID
     * @return true if this < other
     */
    [[nodiscard]] constexpr bool operator<(const TypedId& other) const noexcept {
        return value_ < other.value_;
    }

    /**
     * @brief Get a hash value for use in unordered containers
     * @return Hash value
     */
    [[nodiscard]] constexpr std::size_t hash() const noexcept {
        return static_cast<std::size_t>(value_);
    }

    /**
     * @brief Create an invalid ID
     * @return Invalid ID
     */
    [[nodiscard]] static constexpr TypedId invalid() noexcept {
        return TypedId{};
    }

private:
    ValueType value_;
};

// Type tags for different ID types
struct ResourceIdTag {};
struct AssetIdTag {};
struct TextureIdTag {};
struct MeshIdTag {};
struct AudioIdTag {};

// Specific ID type aliases
using ResourceId = TypedId<ResourceIdTag, std::uint32_t>;
using AssetId = TypedId<AssetIdTag, std::uint32_t>;
using TextureId = TypedId<TextureIdTag, std::uint32_t>;
using MeshId = TypedId<MeshIdTag, std::uint32_t>;
using AudioId = TypedId<AudioIdTag, std::uint32_t>;

} // namespace poko::core

// Enable TypedId as key in std::unordered_map
namespace std {

template<typename Tag, typename UnderlyingType>
struct hash<poko::core::TypedId<Tag, UnderlyingType>> {
    std::size_t operator()(const poko::core::TypedId<Tag, UnderlyingType>& id) const noexcept {
        return id.hash();
    }
};

} // namespace std
