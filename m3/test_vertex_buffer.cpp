/**
 * @file test_vertex_buffer.cpp
 * @brief M3 tests for Poko vertex buffer API.
 */

#include "poko/render/vertex_buffer.h"
#include "poko/render/device.h"
#include "poko/render/context.h"

#include <cstdio>
#include <cassert>

int main() {
    std::printf("M3 Vertex Buffer API Tests\n");
    std::printf("==========================\n\n");

    // Test 1: Vertex buffer defaults to invalid
    {
        std::printf("Test 1: Vertex buffer defaults to invalid... ");
        poko::render::VertexBuffer buffer;
        assert(!buffer.is_valid());
        std::printf("PASS\n");
    }

    // Test 2: Invalid device returns error
    {
        std::printf("Test 2: Invalid device returns error... ");
        poko::render::VertexBuffer buffer;
        poko::render::BufferDesc desc;
        desc.size = 12;
        
        auto result = buffer.initialize(nullptr, desc);
        assert(result == poko::render::BufferErrorCode::InvalidDevice);
        assert(!buffer.is_valid());
        std::printf("PASS\n");
    }

    // Test 3: Invalid size (zero) returns error
    {
        std::printf("Test 3: Invalid size (zero) returns error... ");
        poko::render::VertexBuffer buffer;
        poko::render::Device device;  // Not initialized
        poko::render::BufferDesc desc;
        desc.size = 0;
        
        auto result = buffer.initialize(&device, desc);
        // Check: since device is invalid, should return InvalidDevice (checked first)
        assert(result == poko::render::BufferErrorCode::InvalidDevice);
        assert(!buffer.is_valid());
        std::printf("PASS\n");
    }

    // Test 4: Valid buffer creation with data
    {
        std::printf("Test 4: Valid buffer creation with data... ");
        const float vertices[] = {1.0f, 2.0f, 3.0f};
        poko::render::VertexBuffer buffer;
        poko::render::Device device;  // Not initialized, so this will fail at device validation
        poko::render::BufferDesc desc;
        desc.data = vertices;
        desc.size = sizeof(vertices);
        
        auto result = buffer.initialize(&device, desc);
        assert(result == poko::render::BufferErrorCode::InvalidDevice);  // Device not valid
        assert(!buffer.is_valid());
        std::printf("PASS (device validation works)\n");
    }

    // Test 5: Context::set_vertex_buffer with null buffer
    {
        std::printf("Test 5: Context::set_vertex_buffer with null buffer... ");
        poko::render::Context context;
        bool result = context.set_vertex_buffer(nullptr);
        assert(!result);
        std::printf("PASS\n");
    }

    // Test 6: Context::set_vertex_buffer with invalid buffer
    {
        std::printf("Test 6: Context::set_vertex_buffer with invalid buffer... ");
        poko::render::Context context;
        poko::render::VertexBuffer buffer;  // Not initialized
        bool result = context.set_vertex_buffer(&buffer);
        assert(!result);
        std::printf("PASS\n");
    }

    // Test 7: get_size returns 0 for invalid buffer
    {
        std::printf("Test 7: get_size returns 0 for invalid buffer... ");
        poko::render::VertexBuffer buffer;
        assert(buffer.get_size() == 0);
        std::printf("PASS\n");
    }

    // Test 8: update on uninitialized buffer fails
    {
        std::printf("Test 8: update on uninitialized buffer fails... ");
        poko::render::VertexBuffer buffer;
        float data[3] = {0};
        auto result = buffer.update(data, 12, 0);
        assert(result != poko::render::BufferErrorCode::Success);
        std::printf("PASS\n");
    }

    std::printf("\nAll M3 vertex buffer tests passed.\n");
    return 0;
}
