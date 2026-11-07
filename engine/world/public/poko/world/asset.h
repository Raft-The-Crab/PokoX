/**
 * @file asset.h
 * @brief Asset import system for scene graph.
 *
 * Provides asset loading and management for the world module.
 *
 * Production Status: M1 World Foundation (Production)
 */

#ifndef POKO_WORLD_ASSET_H
#define POKO_WORLD_ASSET_H

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "poko/math/vec3.h"
#include "poko/math/vec4.h"
#include "poko/math/mat4.h"

namespace poko::world {

/**
 * @brief Asset types.
 */
enum class AssetType {
    Unknown,
    Texture,
    Mesh,
    Material,
    Shader,
    Audio,
    Script,
    Model,
    Animation,
    Prefab
};

/**
 * @brief Asset handle - safe reference to an asset.
 */
class AssetHandle {
public:
    AssetHandle() : id_(0) {}
    explicit AssetHandle(uint64_t id) : id_(id) {}

    bool is_valid() const { return id_ != 0; }
    uint64_t id() const { return id_; }

    bool operator==(const AssetHandle& other) const { return id_ == other.id_; }
    bool operator!=(const AssetHandle& other) const { return id_ != other.id_; }

private:
    uint64_t id_;
};

/**
 * @brief Asset metadata.
 */
struct AssetMetadata {
    AssetType type = AssetType::Unknown;
    std::string path;
    uint64_t size = 0;
    uint64_t hash = 0;
    std::vector<std::string> dependencies;
    uint32_t texture_width = 0;   // For textures
    uint32_t texture_height = 0;  // For textures
    math::Vec4 color;             // For materials
    math::Mat4 transform;         // For models/prefabs
};

/**
 * @brief Asset interface - loaded asset data.
 */
class Asset {
public:
    Asset() = default;
    virtual ~Asset() = default;

    // Non-copyable, non-movable
    Asset(const Asset&) = delete;
    Asset& operator=(const Asset&) = delete;
    Asset(Asset&&) = delete;
    Asset& operator=(Asset&&) = delete;

    /**
     * @brief Gets the asset handle.
     */
    AssetHandle get_handle() const { return handle_; }

    /**
     * @brief Gets the asset metadata.
     */
    const AssetMetadata& get_metadata() const { return metadata_; }

    /**
     * @brief Gets the asset data.
     */
    virtual const void* get_data() const = 0;

    /**
     * @brief Gets the asset data size.
     */
    virtual size_t get_data_size() const = 0;

    /**
     * @brief Gets the asset type.
     */
    AssetType get_type() const { return metadata_.type; }

    /**
     * @brief Checks if asset is loaded.
     */
    bool is_loaded() const { return handle_.is_valid(); }

protected:
    AssetHandle handle_;
    AssetMetadata metadata_;
};

/**
 * @brief Asset manager - loads and manages assets.
 */
class AssetManager {
public:
    AssetManager();
    ~AssetManager();

    // Non-copyable, non-movable
    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;
    AssetManager(AssetManager&&) = delete;
    AssetManager& operator=(AssetManager&&) = delete;

    /**
     * @brief Initializes the asset manager.
     */
    bool initialize();

    /**
     * @brief Loads an asset from file.
     *
     * @param path Asset file path.
     * @return Asset handle or invalid handle on failure.
     */
    AssetHandle load_asset(const std::string& path);

    /**
     * @brief Loads a texture asset.
     *
     * @param path Texture file path.
     * @param width Texture width.
     * @param height Texture height.
     * @param format Texture format (e.g., "RGBA8").
     * @return Asset handle or invalid handle on failure.
     */
    AssetHandle load_texture(const std::string& path, uint32_t width, uint32_t height, const std::string& format);

    /**
     * @brief Loads a model asset.
     *
     * @param path Model file path.
     * @return Asset handle or invalid handle on failure.
     */
    AssetHandle load_model(const std::string& path);

    /**
     * @brief Gets an asset by handle.
     *
     * @param handle Asset handle.
     * @return Pointer to asset or nullptr if not found.
     */
    Asset* get_asset(AssetHandle handle);

    /**
     * @brief Gets an asset by type.
     *
     * @param type Asset type.
     * @return Vector of asset handles of the specified type.
     */
    std::vector<AssetHandle> get_assets_by_type(AssetType type) const;

    /**
     * @brief Unloads an asset.
     *
     * @param handle Asset handle.
     * @return true on success, false on failure.
     */
    bool unload_asset(AssetHandle handle);

    /**
     * @brief Gets all loaded assets.
     */
    const std::vector<AssetHandle>& get_loaded_assets() const;

    /**
     * @brief Gets the number of loaded assets.
     */
    size_t get_asset_count() const;

private:
    AssetHandle load_internal(const std::string& path, AssetType forced_type,
                              uint32_t texture_width, uint32_t texture_height,
                              const std::string& format);

    std::vector<std::unique_ptr<Asset>> assets_;
    std::vector<AssetHandle> loaded_handles_;
    std::unordered_map<uint64_t, size_t> asset_index_;
    std::unordered_map<std::string, AssetHandle> path_index_;
    uint64_t next_asset_id_;
};

} // namespace poko::world

#endif // POKO_WORLD_ASSET_H
