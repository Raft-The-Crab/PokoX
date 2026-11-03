/**
 * @file texture_impl.cpp
 * @brief Diligent Core implementation of Texture.
 */

#include "backend/diligent/internal/texture_impl.h"
#include "backend/diligent/internal/device_impl.h"

#include <cstdio>
#include <cstring>

namespace poko::render {

Texture::Impl::~Impl() {
    if (srv) {
        srv->Release();
        srv = nullptr;
    }
    if (texture) {
        texture->Release();
        texture = nullptr;
    }
}

bool Texture::Impl::initialize(Device* device, const TextureDesc& desc) {
    if (!device || !device->is_valid()) {
        std::fprintf(stderr, "Invalid device\n");
        return false;
    }

    auto device_impl = get_device_impl(device);
    if (!device_impl || !device_impl->device) {
        std::fprintf(stderr, "Invalid device implementation\n");
        return false;
    }

    this->device = device;
    this->desc = desc;

    // Validate inputs
    if (!desc.name) {
        std::fprintf(stderr, "Texture must have a name\n");
        return false;
    }
    if (desc.width == 0 || desc.height == 0) {
        std::fprintf(stderr, "Texture '%s': Invalid dimensions (%u x %u)\n", desc.name, desc.width, desc.height);
        return false;
    }
    if (desc.type != TextureType::Texture2D) {
        std::fprintf(stderr, "Texture '%s': Invalid type: %d\n", desc.name, static_cast<int>(desc.type));
        return false;
    }
    // Validate texture size limits (typical OpenGL 4.4 limits)
    if (desc.width > 16384 || desc.height > 16384) {
        std::fprintf(stderr, "Texture '%s': Dimensions exceed maximum (16384 x 16384)\n", desc.name);
        return false;
    }
    if (desc.data_size > 0 && !desc.data) {
        std::fprintf(stderr, "Texture '%s': Null data pointer with non-zero size\n", desc.name);
        return false;
    }
    // Validate data size matches dimensions for RGBA8
    uint32_t expected_size = desc.width * desc.height * 4;  // RGBA8 = 4 bytes per pixel
    if (desc.data && desc.data_size != expected_size) {
        std::fprintf(stderr, "Data size (%u) does not match expected size for dimensions (%u)\n", 
            desc.data_size, expected_size);
        return false;
    }

    // Copy data if provided
    if (desc.data && desc.data_size > 0) {
        data.resize(desc.data_size);
        std::memcpy(data.data(), desc.data, desc.data_size);
    }

    // Map Poko format to Diligent format
    Diligent::TEXTURE_FORMAT format = Diligent::TEX_FORMAT_RGBA8_UNORM;
    switch (desc.format) {
        case TextureFormat::RGBA8_UNORM:
            format = Diligent::TEX_FORMAT_RGBA8_UNORM;
            break;
        case TextureFormat::RGBA8_SRGB:
            format = Diligent::TEX_FORMAT_RGBA8_UNORM_SRGB;
            break;
        case TextureFormat::RG8_UNORM:
            format = Diligent::TEX_FORMAT_RG8_UNORM;
            break;
        case TextureFormat::R8_UNORM:
            format = Diligent::TEX_FORMAT_R8_UNORM;
            break;
        case TextureFormat::RGBA16_FLOAT:
            format = Diligent::TEX_FORMAT_RGBA16_FLOAT;
            break;
        case TextureFormat::RGBA16_UNORM:
            format = Diligent::TEX_FORMAT_RGBA16_UNORM;
            break;
        case TextureFormat::RGBA32_FLOAT:
            format = Diligent::TEX_FORMAT_RGBA32_FLOAT;
            break;
        case TextureFormat::D16_UNORM:
            format = Diligent::TEX_FORMAT_D16_UNORM;
            break;
        case TextureFormat::D24_UNORM_S8_UINT:
            format = Diligent::TEX_FORMAT_D24_UNORM_S8_UINT;
            break;
        case TextureFormat::D32_FLOAT:
            format = Diligent::TEX_FORMAT_D32_FLOAT;
            break;
        case TextureFormat::D32_FLOAT_S8_UINT:
            format = Diligent::TEX_FORMAT_D32_FLOAT_S8X24_UINT;
            break;
        default:
            format = Diligent::TEX_FORMAT_RGBA8_UNORM;
            break;
    }

    // Create Diligent texture
    Diligent::TextureDesc tex_desc;
    tex_desc.Name = desc.name ? desc.name : "Poko Texture";
    tex_desc.Type = Diligent::RESOURCE_DIM_TEX_2D;
    tex_desc.Width = desc.width;
    tex_desc.Height = desc.height;
    tex_desc.MipLevels = desc.mip_levels;
    tex_desc.Format = format;
    tex_desc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
    tex_desc.Usage = Diligent::USAGE_IMMUTABLE;

    Diligent::TextureData tex_data;
    tex_data.pSubResources = nullptr;
    tex_data.NumSubresources = 0;

    if (!data.empty()) {
        Diligent::TextureSubResData sub_res;
        sub_res.pData = data.data();
        sub_res.Stride = desc.width * 4;  // RGBA8 = 4 bytes per pixel
        sub_res.DepthStride = 0;
        
        tex_data.pSubResources = &sub_res;
        tex_data.NumSubresources = 1;
    }

    device_impl->device->CreateTexture(tex_desc, &tex_data, &texture);

    if (!texture) {
        std::fprintf(stderr, "Failed to create texture\n");
        return false;
    }

    // Create shader resource view - deferred to M6 material creation
    // SRV is created on-demand when material binds the texture
    srv = nullptr;
    
    return true;
}

} // namespace poko::render
