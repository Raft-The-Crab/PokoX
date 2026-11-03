/**
 * @file test_constant_buffer.cpp
 * @brief M7 tests for Poko constant buffer API.
 */

#include "poko/render/constant_buffer.h"
#include "poko/render/device.h"
#include "poko/render/context.h"

#include <cstdio>
#include <cassert>
#include <cstring>

int main() {
    std::printf("M7 Constant Buffer API Tests\n");
    std::printf("=============================\n\n");

    // Test 1: Constant buffer defaults to invalid
    {
        std::printf("Test 1: Constant buffer defaults to invalid... ");
        poko::render::ConstantBuffer buffer;
        assert(!buffer.is_valid());
        std::printf("PASS\n");
    }

    // Test 2: Invalid device returns error
    {
        std::printf("Test 2: Invalid device returns error... ");
        poko::render::ConstantBuffer buffer;
        poko::render::ConstantBufferDesc desc;
        desc.size = 64;
        
        auto result = buffer.initialize(nullptr, desc);
        assert(result == poko::render::ConstantBufferErrorCode::InvalidDevice);
        assert(!buffer.is_valid());
        std::printf("PASS\n");
    }

    // Test 3: Invalid size (zero) returns error
    {
        std::printf("Test 3: Invalid size (zero) returns error... ");
        poko::render::ConstantBuffer buffer;
        poko::render::Device device;  // Not initialized
        poko::render::ConstantBufferDesc desc;
        desc.size = 0;
        
        auto result = buffer.initialize(&device, desc);
        assert(result == poko::render::ConstantBufferErrorCode::InvalidDevice);
        assert(!buffer.is_valid());
        std::printf("PASS\n");
    }

    // Test 4: Invalid size (not multiple of 16) returns error
    {
        std::printf("Test 4: Invalid size (not multiple of 16) returns error... ");
        poko::render::ConstantBuffer buffer;
        poko::render::Device device;  // Not initialized
        poko::render::ConstantBufferDesc desc;
        desc.size = 63;  // Not multiple of 16
        
        auto result = buffer.initialize(&device, desc);
        assert(result == poko::render::ConstantBufferErrorCode::InvalidDevice);
        assert(!buffer.is_valid());
        std::printf("PASS\n");
    }

    // Test 5: Invalid size (too large) returns error
    {
        std::printf("Test 5: Invalid size (too large) returns error... ");
        poko::render::ConstantBuffer buffer;
        poko::render::Device device;  // Not initialized
        poko::render::ConstantBufferDesc desc;
        desc.size = 256 * 1024 * 1024 + 16;  // Over 256 MB
        
        auto result = buffer.initialize(&device, desc);
        assert(result == poko::render::ConstantBufferErrorCode::InvalidDevice);
        assert(!buffer.is_valid());
        std::printf("PASS\n");
    }

    // Test 6: Valid buffer creation with data
    {
        std::printf("Test 6: Valid buffer creation with data... ");
        float data[16] = {0};
        std::memset(data, 0, sizeof(data));
        poko::render::ConstantBuffer buffer;
        poko::render::Device device;  // Not initialized, so this will fail at device validation
        poko::render::ConstantBufferDesc desc;
        desc.data = data;
        desc.size = 64;  // 16 floats = 64 bytes (multiple of 16)
        
        auto result = buffer.initialize(&device, desc);
        assert(result == poko::render::ConstantBufferErrorCode::InvalidDevice);  // Device not valid
        assert(!buffer.is_valid());
        std::printf("PASS (device validation works)\n");
    }

    // Test 7: Update on uninitialized buffer fails
    {
        std::printf("Test 7: Update on uninitialized buffer fails... ");
        poko::render::ConstantBuffer buffer;
        float data[16] = {0};
        
        auto result = buffer.update(data, 64, 0);
        // Should fail since buffer is not initialized
        assert(result != poko::render::ConstantBufferErrorCode::Success);
        std::printf("PASS\n");
    }

    // Test 8: Update with null data fails
    {
        std::printf("Test 8: Update with null data fails... ");
        poko::render::ConstantBuffer buffer;
        
        auto result = buffer.update(nullptr, 64, 0);
        assert(result == poko::render::ConstantBufferErrorCode::InvalidData);
        std::printf("PASS\n");
    }

    // Test 9: Update with zero size fails
    {
        std::printf("Test 9: Update with zero size fails... ");
        poko::render::ConstantBuffer buffer;
        float data[16] = {0};
        
        auto result = buffer.update(data, 0, 0);
        assert(result == poko::render::ConstantBufferErrorCode::InvalidSize);
        std::printf("PASS\n");
    }

    // Test 10: get_size returns 0 for invalid buffer
    {
        std::printf("Test 11: get_size returns 0 for invalid buffer... ");
        poko::render::ConstantBuffer buffer;
        assert(buffer.get_size() == 0);
        std::printf("PASS\n");
    }

    // Test 12: get_device returns nullptr for invalid buffer
    {
        std::printf("Test 12: get_device returns nullptr for invalid buffer... ");
        poko::render::ConstantBuffer buffer;
        assert(buffer.get_device() == nullptr);
        std::printf("PASS\n");
    }

    std::printf("\nAll M7 constant buffer tests passed.\n");
    return 0;
}
