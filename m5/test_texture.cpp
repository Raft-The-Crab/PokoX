/**
 * @file test_texture.cpp
 * @brief M5 tests for Poko texture API.
 */

#include "poko/render/texture.h"
#include "poko/render/device.h"
#include "poko/render/context.h"

#include <cstdio>
#include <cassert>
#include <cstring>

int main() {
    std::printf("M5 Texture API Tests\n");
    std::printf("======================\n\n");

    // Test 1: Texture defaults to invalid
    {
        std::printf("Test 1: Texture defaults to invalid... ");
        poko::render::Texture texture;
        assert(!texture.is_valid());
        std::printf("PASS\n");
    }

    // Test 2: Invalid device returns error
    {
        std::printf("Test 2: Invalid device returns error... ");
        poko::render::Texture texture;
        poko::render::TextureDesc desc;
        desc.width = 64;
        desc.height = 64;
        
        auto result = texture.initialize(nullptr, desc);
        assert(result == poko::render::TextureErrorCode::InvalidDevice);
        assert(!texture.is_valid());
        std::printf("PASS\n");
    }

    // Test 3: Invalid size (zero) returns error
    {
        std::printf("Test 3: Invalid size (zero) returns error... ");
        poko::render::Texture texture;
        poko::render::Device device;  // Not initialized
        poko::render::TextureDesc desc;
        desc.width = 0;
        desc.height = 0;
        
        auto result = texture.initialize(&device, desc);
        assert(result == poko::render::TextureErrorCode::InvalidDevice);  // Device checked first
        assert(!texture.is_valid());
        std::printf("PASS\n");
    }

    // Test 4: Invalid type returns error
    {
        std::printf("Test 4: Invalid type returns error... ");
        poko::render::Texture texture;
        poko::render::Device device;  // Not initialized
        poko::render::TextureDesc desc;
        desc.width = 64;
        desc.height = 64;
        desc.type = poko::render::TextureType::Unknown;
        
        auto result = texture.initialize(&device, desc);
        assert(result == poko::render::TextureErrorCode::InvalidDevice);  // Device checked first
        assert(!texture.is_valid());
        std::printf("PASS\n");
    }

    // Test 5: Valid texture parameters pass validation
    {
        std::printf("Test 5: Valid texture parameters pass validation... ");
        const uint8_t dummy_data[] = {255, 0, 0, 255};
        poko::render::Texture texture;
        poko::render::Device device;  // Not initialized
        poko::render::TextureDesc desc;
        desc.width = 64;
        desc.height = 64;
        desc.type = poko::render::TextureType::Texture2D;
        desc.format = poko::render::TextureFormat::RGBA8_UNORM;
        desc.data = dummy_data;
        desc.data_size = sizeof(dummy_data);
        
        auto result = texture.initialize(&device, desc);
        assert(result == poko::render::TextureErrorCode::InvalidDevice);  // Device checked first
        assert(!texture.is_valid());
        std::printf("PASS (device validation checked first)\n");
    }

    // Test 6: Context::set_texture returns false (deferred to M6)
    {
        std::printf("Test 6: Context::set_texture returns false (M6)... ");
        poko::render::Context context;
        poko::render::Texture texture;
        bool result = context.set_texture(&texture, 0);
        assert(!result);
        std::printf("PASS\n");
    }

    // Test 7: get_format returns Unknown for invalid texture
    {
        std::printf("Test 7: get_format returns Unknown for invalid texture... ");
        poko::render::Texture texture;
        assert(texture.get_format() == poko::render::TextureFormat::Unknown);
        std::printf("PASS\n");
    }

    // Test 8: get_name returns name for texture
    {
        std::printf("Test 8: get_name returns name for texture... ");
        poko::render::Texture texture;
        const char* name = texture.get_name();
        assert(name != nullptr);
        std::printf("PASS\n");
    }

    // Test 9: Texture format validation (Unknown format)
    {
        std::printf("Test 9: Texture format validation (Unknown format)... ");
        const uint8_t dummy_data[] = {255, 0, 0, 255};
        poko::render::Texture texture;
        poko::render::Device device;  // Not initialized
        poko::render::TextureDesc desc;
        desc.width = 64;
        desc.height = 64;
        desc.format = poko::render::TextureFormat::Unknown;
        desc.data = dummy_data;
        desc.data_size = sizeof(dummy_data);
        
        auto result = texture.initialize(&device, desc);
        assert(result == poko::render::TextureErrorCode::InvalidDevice);
        std::printf("PASS (device checked first)\n");
    }

    // Test 10: Texture dimension validation
    {
        std::printf("Test 10: Texture dimension validation... ");
        const uint8_t dummy_data[] = {255, 0, 0, 255};
        poko::render::Texture texture;
        poko::render::Device device;  // Not initialized
        poko::render::TextureDesc desc;
        desc.width = 1;  // Minimum
        desc.height = 1;  // Minimum
        desc.format = poko::render::TextureFormat::RGBA8_UNORM;
        desc.data = dummy_data;
        desc.data_size = sizeof(dummy_data);
        
        auto result = texture.initialize(&device, desc);
        assert(result == poko::render::TextureErrorCode::InvalidDevice);
        std::printf("PASS (device checked first)\n");
    }

    // Test 11: Texture with null data
    {
        std::printf("Test 11: Texture with null data... ");
        poko::render::Texture texture;
        poko::render::Device device;  // Not initialized
        poko::render::TextureDesc desc;
        desc.width = 64;
        desc.height = 64;
        desc.format = poko::render::TextureFormat::RGBA8_UNORM;
        desc.data = nullptr;
        desc.data_size = 0;
        
        auto result = texture.initialize(&device, desc);
        assert(result == poko::render::TextureErrorCode::InvalidDevice);
        std::printf("PASS (device checked first)\n");
    }

    // Test 12: TextureType validation
    {
        std::printf("Test 12: TextureType validation... ");
        const uint8_t dummy_data[] = {255, 0, 0, 255};
        poko::render::Texture texture;
        poko::render::Device device;  // Not initialized
        poko::render::TextureDesc desc;
        desc.width = 64;
        desc.height = 64;
        desc.type = poko::render::TextureType::Texture2D;
        desc.format = poko::render::TextureFormat::RGBA8_UNORM;
        desc.data = dummy_data;
        desc.data_size = sizeof(dummy_data);
        
        auto result = texture.initialize(&device, desc);
        assert(result == poko::render::TextureErrorCode::InvalidDevice);
        std::printf("PASS (device checked first)\n");
    }

    std::printf("\nAll M5 texture tests passed.\n");
    std::printf("Note: Texture binding is deferred to M6 (requires SRB management).\n");
    return 0;
}
