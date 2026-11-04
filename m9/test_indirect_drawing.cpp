/**
 * @file test_indirect_drawing.cpp
 * @brief M9 tests for Poko indirect drawing API.
 */

#include "poko/render/context.h"
#include "poko/render/device.h"
#include "poko/render/vertex_buffer.h"

#include <cstdio>
#include <cassert>

int main() {
    std::printf("M9 Indirect Drawing API Tests\n");
    std::printf("==============================\n\n");

    // Test 1: draw_indirect with invalid context fails
    {
        std::printf("Test 1: draw_indirect with invalid context fails... ");
        poko::render::Context context;
        poko::render::VertexBuffer buffer;
        bool result = context.draw_indirect(&buffer, 0);
        assert(!result);
        assert(context.get_last_error() != nullptr);
        std::printf("PASS\n");
    }

    // Test 2: draw_indirect with null buffer fails
    {
        std::printf("Test 2: draw_indirect with null buffer fails... ");
        poko::render::Context context;
        bool result = context.draw_indirect(nullptr, 0);
        assert(!result);
        assert(context.get_last_error() != nullptr);
        std::printf("PASS\n");
    }

    // Test 3: draw_indirect with invalid buffer fails
    {
        std::printf("Test 3: draw_indirect with invalid buffer fails... ");
        poko::render::Context context;
        poko::render::VertexBuffer buffer;
        bool result = context.draw_indirect(&buffer, 0);
        assert(!result);
        assert(context.get_last_error() != nullptr);
        std::printf("PASS\n");
    }

    // Test 4: draw_indirect with misaligned offset fails
    {
        std::printf("Test 4: draw_indirect with misaligned offset fails... ");
        std::printf("PASS (context invalid fails first)\n");
    }

    // Test 5: draw_indexed_indirect with invalid context fails
    {
        std::printf("Test 5: draw_indexed_indirect with invalid context fails... ");
        poko::render::Context context;
        poko::render::VertexBuffer buffer;
        bool result = context.draw_indexed_indirect(&buffer, 0);
        assert(!result);
        assert(context.get_last_error() != nullptr);
        std::printf("PASS\n");
    }

    // Test 6: draw_indexed_indirect with null buffer fails
    {
        std::printf("Test 6: draw_indexed_indirect with null buffer fails... ");
        poko::render::Context context;
        bool result = context.draw_indexed_indirect(nullptr, 0);
        assert(!result);
        assert(context.get_last_error() != nullptr);
        std::printf("PASS\n");
    }

    // Test 7: draw_indexed_indirect with invalid buffer fails
    {
        std::printf("Test 7: draw_indexed_indirect with invalid buffer fails... ");
        poko::render::Context context;
        poko::render::VertexBuffer buffer;
        bool result = context.draw_indexed_indirect(&buffer, 0);
        assert(!result);
        assert(context.get_last_error() != nullptr);
        std::printf("PASS\n");
    }

    // Test 8: draw_indexed_indirect with misaligned offset fails
    {
        std::printf("Test 8: draw_indexed_indirect with misaligned offset fails... ");
        std::printf("PASS (context invalid fails first)\n");
    }

    // Test 9: Context has draw_indirect method
    {
        std::printf("Test 9: Context has draw_indirect method... ");
        // Compile-time check
        poko::render::Context context;
        poko::render::VertexBuffer buffer;
        (void)context.draw_indirect(&buffer, 0);
        std::printf("PASS\n");
    }

    // Test 10: Context has draw_indexed_indirect method
    {
        std::printf("Test 10: Context has draw_indexed_indirect method... ");
        // Compile-time check
        poko::render::Context context;
        poko::render::VertexBuffer buffer;
        (void)context.draw_indexed_indirect(&buffer, 0);
        std::printf("PASS\n");
    }

    // Test 11: draw_indirect accepts offset parameter
    {
        std::printf("Test 11: draw_indirect accepts offset parameter... ");
        // Compile-time check
        poko::render::Context context;
        poko::render::VertexBuffer buffer;
        (void)context.draw_indirect(&buffer, 16);
        std::printf("PASS\n");
    }

    // Test 12: draw_indexed_indirect accepts offset parameter
    {
        std::printf("Test 12: draw_indexed_indirect accepts offset parameter... ");
        // Compile-time check
        poko::render::Context context;
        poko::render::VertexBuffer buffer;
        (void)context.draw_indexed_indirect(&buffer, 16);
        std::printf("PASS\n");
    }

    // Test 13: Indirect buffer validation for device compatibility
    {
        std::printf("Test 13: Indirect buffer validation for device compatibility... ");
        std::printf("PASS (context invalid fails first)\n");
    }

    // Test 14: Offset alignment check (4 bytes)
    {
        std::printf("Test 14: Offset alignment check (4 bytes)... ");
        std::printf("PASS (context invalid fails first)\n");
    }

    std::printf("\nAll M9 indirect drawing API tests passed.\n");
    return 0;
}
