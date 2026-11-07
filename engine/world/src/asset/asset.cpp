/**
 * @file asset.cpp
 * @brief Asset import system implementation.
 *
 * Production Status: M1 World Foundation (Production)
 */

#include "poko/world/asset.h"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <string>
#include <utility>

namespace poko::world {

namespace {

// Maximum asset size accepted by the manager (256 MiB). Larger files are
// rejected up front instead of being read into memory.
constexpr std::streamoff kMaxAssetSize = 256ll * 1024 * 1024;

// Lexically normalize an asset path so that trivially equivalent spellings
// ("./a.png", "a\\b.png", "a//b.png") deduplicate to the same asset. This is
// not a filesystem canonicalization; it does not resolve ".." or symlinks.
std::string normalize_asset_path(const std::string& path) {
    std::string p = path;
    std::replace(p.begin(), p.end(), '\\', '/');

    // Collapse duplicate slashes and remove "/./" segments
    std::string out;
    out.reserve(p.size());
    size_t i = 0;
    while (i < p.size()) {
        if (p[i] == '/') {
            if (!out.empty() && out.back() == '/') {
                ++i;
                continue;
            }
            // Skip "/./" segments
            if (i + 2 < p.size() && p[i + 1] == '.' && p[i + 2] == '/') {
                i += 3;
                continue;
            }
        }
        out.push_back(p[i]);
        ++i;
    }

    // Strip a single leading "./"
    if (out.size() >= 2 && out[0] == '.' && out[1] == '/') {
        out.erase(0, 2);
    }
    return out;
}

std::string path_extension(const std::string& path) {
    size_t dot = path.find_last_of('.');
    size_t slash = path.find_last_of('/');
    if (dot == std::string::npos || (slash != std::string::npos && dot < slash)) {
        return "";
    }
    std::string ext = path.substr(dot + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return ext;
}

AssetType detect_asset_type(const std::string& extension) {
    if (extension == "png" || extension == "jpg" || extension == "jpeg" ||
        extension == "bmp" || extension == "tga" || extension == "dds" ||
        extension == "hdr" || extension == "exr") {
        return AssetType::Texture;
    }
    if (extension == "obj" || extension == "fbx" || extension == "gltf" ||
        extension == "glb" || extension == "dae") {
        return AssetType::Model;
    }
    if (extension == "wav" || extension == "mp3" || extension == "ogg" ||
        extension == "flac") {
        return AssetType::Audio;
    }
    if (extension == "glsl" || extension == "hlsl" || extension == "vert" ||
        extension == "frag" || extension == "comp" || extension == "spv") {
        return AssetType::Shader;
    }
    if (extension == "lua" || extension == "py" || extension == "js" ||
        extension == "cs") {
        return AssetType::Script;
    }
    if (extension == "prefab" || extension == "scene") {
        return AssetType::Prefab;
    }
    if (extension == "anim" || extension == "animation" || extension == "gltfanim") {
        return AssetType::Animation;
    }
    if (extension == "mat" || extension == "material") {
        return AssetType::Material;
    }
    return AssetType::Unknown;
}

// FNV-1a 64-bit hash over asset bytes, stored in metadata for change
// detection and cache validation.
uint64_t fnv1a(const std::vector<uint8_t>& data) {
    uint64_t hash = 14695981039346656037ull;
    for (uint8_t byte : data) {
        hash ^= byte;
        hash *= 1099511628211ull;
    }
    return hash;
}

// Simple file asset implementation
class FileAsset : public Asset {
public:
    FileAsset(AssetHandle handle, const AssetMetadata& metadata, std::vector<uint8_t>&& data)
        : data_(std::move(data)) {
        handle_ = handle;
        metadata_ = metadata;
    }

    const void* get_data() const override { return data_.data(); }
    size_t get_data_size() const override { return data_.size(); }

private:
    std::vector<uint8_t> data_;
};

// Texture asset implementation
class TextureAsset : public Asset {
public:
    TextureAsset(AssetHandle handle, const AssetMetadata& metadata, std::vector<uint8_t>&& data)
        : data_(std::move(data)) {
        handle_ = handle;
        metadata_ = metadata;
    }

    const void* get_data() const override { return data_.data(); }
    size_t get_data_size() const override { return data_.size(); }

private:
    std::vector<uint8_t> data_;
};

// Model asset implementation
class ModelAsset : public Asset {
public:
    ModelAsset(AssetHandle handle, const AssetMetadata& metadata, std::vector<uint8_t>&& data)
        : data_(std::move(data)) {
        handle_ = handle;
        metadata_ = metadata;
    }

    const void* get_data() const override { return data_.data(); }
    size_t get_data_size() const override { return data_.size(); }

private:
    std::vector<uint8_t> data_;
};

} // namespace

AssetManager::AssetManager() : next_asset_id_(1) {}

AssetManager::~AssetManager() = default;

bool AssetManager::initialize() {
    return true;
}

AssetHandle AssetManager::load_internal(const std::string& path, AssetType forced_type,
                                        uint32_t texture_width, uint32_t texture_height,
                                        const std::string& format) {
    (void)format;  // Reserved for future compressed-format metadata
    const std::string normalized = normalize_asset_path(path);
    if (normalized.empty()) {
        return AssetHandle();
    }

    // Deduplicate: any loader returning an already-loaded path yields the
    // existing handle instead of re-reading the file.
    auto existing = path_index_.find(normalized);
    if (existing != path_index_.end()) {
        return existing->second;
    }

    // Size check before reading so oversized files never enter memory
    std::ifstream probe(path, std::ios::binary | std::ios::ate);
    if (!probe.is_open()) {
        return AssetHandle();
    }
    std::streamoff file_size = probe.tellg();
    if (file_size <= 0 || file_size > kMaxAssetSize) {
        return AssetHandle();
    }

    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return AssetHandle();
    }
    std::vector<uint8_t> data(static_cast<size_t>(file_size));
    file.read(reinterpret_cast<char*>(data.data()), file_size);
    if (!file || file.gcount() != file_size) {
        return AssetHandle();
    }

    AssetMetadata metadata;
    metadata.path = normalized;
    metadata.size = static_cast<uint64_t>(data.size());
    metadata.hash = fnv1a(data);
    metadata.type = (forced_type != AssetType::Unknown)
                        ? forced_type
                        : detect_asset_type(path_extension(normalized));
    metadata.texture_width = texture_width;
    metadata.texture_height = texture_height;

    AssetHandle handle(next_asset_id_++);
    std::unique_ptr<Asset> asset;

    switch (metadata.type) {
        case AssetType::Texture:
            asset = std::make_unique<TextureAsset>(handle, metadata, std::move(data));
            break;
        case AssetType::Model:
            asset = std::make_unique<ModelAsset>(handle, metadata, std::move(data));
            break;
        default:
            asset = std::make_unique<FileAsset>(handle, metadata, std::move(data));
            break;
    }

    const size_t index = assets_.size();
    assets_.push_back(std::move(asset));
    loaded_handles_.push_back(handle);
    asset_index_.emplace(handle.id(), index);
    path_index_.emplace(normalized, handle);

    return handle;
}

AssetHandle AssetManager::load_asset(const std::string& path) {
    return load_internal(path, AssetType::Unknown, 0, 0, "");
}

AssetHandle AssetManager::load_texture(const std::string& path, uint32_t width, uint32_t height,
                                       const std::string& format) {
    return load_internal(path, AssetType::Texture, width, height, format);
}

AssetHandle AssetManager::load_model(const std::string& path) {
    return load_internal(path, AssetType::Model, 0, 0, "");
}

Asset* AssetManager::get_asset(AssetHandle handle) {
    auto it = asset_index_.find(handle.id());
    if (it == asset_index_.end()) {
        return nullptr;
    }
    return assets_[it->second].get();
}

std::vector<AssetHandle> AssetManager::get_assets_by_type(AssetType type) const {
    std::vector<AssetHandle> result;
    for (const auto& asset : assets_) {
        if (asset->get_type() == type) {
            result.push_back(asset->get_handle());
        }
    }
    return result;
}

bool AssetManager::unload_asset(AssetHandle handle) {
    auto index_it = asset_index_.find(handle.id());
    if (index_it == asset_index_.end()) {
        return false;
    }
    const size_t index = index_it->second;

    // Remove path mapping so the file can be reloaded later
    path_index_.erase(assets_[index]->get_metadata().path);
    asset_index_.erase(index_it);

    // Swap-remove keeps unload O(1); fix up the moved element's index
    if (index != assets_.size() - 1) {
        assets_[index] = std::move(assets_.back());
        asset_index_[assets_[index]->get_handle().id()] = index;
    }
    assets_.pop_back();

    auto handle_it = std::find(loaded_handles_.begin(), loaded_handles_.end(), handle);
    if (handle_it != loaded_handles_.end()) {
        loaded_handles_.erase(handle_it);
    }

    return true;
}

const std::vector<AssetHandle>& AssetManager::get_loaded_assets() const {
    return loaded_handles_;
}

size_t AssetManager::get_asset_count() const {
    return assets_.size();
}

} // namespace poko::world
