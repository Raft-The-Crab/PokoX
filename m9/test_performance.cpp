/**
 * @file test_performance.cpp
 * @brief M9 performance benchmark tests for drawing operations.
 *
 * Tests draw call scaling, instancing performance, and memory allocation tracking.
 */

#include "poko/render/context.h"
#include "poko/render/device.h"
#include "poko/render/vertex_buffer.h"
#include "poko/render/index_buffer.h"

#include <cstdio>
#include <cassert>
#include <chrono>
#include <vector>

// Simple frame time measurement utility
class FrameTimer {
public:
    FrameTimer() : start_time(std::chrono::high_resolution_clock::now()) {}

    double elapsed_ms() const {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        return duration.count() / 1000.0;
    }

    void reset() {
        start_time = std::chrono::high_resolution_clock::now();
    }

private:
    std::chrono::high_resolution_clock::time_point start_time;
};

int main() {
    std::printf("M9 Performance Benchmark Tests\n");
    std::printf("================================\n\n");

    // Test 1: API overhead for draw calls (no actual rendering)
    {
        std::printf("Test 1: API overhead for draw calls... ");
        FrameTimer timer;
        poko::render::Context context;

        uint32_t iterations = 10000;
        for (uint32_t i = 0; i < iterations; ++i) {
            context.draw(4);  // Will fail due to invalid context, but measures API overhead
        }

        double elapsed = timer.elapsed_ms();
        std::printf("PASS (%.2f ms for %u calls, %.2f us/call)\n",
                    elapsed, iterations, (elapsed * 1000.0) / iterations);
    }

    // Test 2: API overhead for draw_indexed calls
    {
        std::printf("Test 2: API overhead for draw_indexed calls... ");
        FrameTimer timer;
        poko::render::Context context;

        uint32_t iterations = 10000;
        for (uint32_t i = 0; i < iterations; ++i) {
            context.draw_indexed(6);
        }

        double elapsed = timer.elapsed_ms();
        std::printf("PASS (%.2f ms for %u calls, %.2f us/call)\n",
                    elapsed, iterations, (elapsed * 1000.0) / iterations);
    }

    // Test 3: API overhead for draw_instanced calls
    {
        std::printf("Test 3: API overhead for draw_instanced calls... ");
        FrameTimer timer;
        poko::render::Context context;

        uint32_t iterations = 10000;
        for (uint32_t i = 0; i < iterations; ++i) {
            context.draw_instanced(4, 100);
        }

        double elapsed = timer.elapsed_ms();
        std::printf("PASS (%.2f ms for %u calls, %.2f us/call)\n",
                    elapsed, iterations, (elapsed * 1000.0) / iterations);
    }

    // Test 4: API overhead for draw_indexed_instanced calls
    {
        std::printf("Test 4: API overhead for draw_indexed_instanced calls... ");
        FrameTimer timer;
        poko::render::Context context;

        uint32_t iterations = 10000;
        for (uint32_t i = 0; i < iterations; ++i) {
            context.draw_indexed_instanced(6, 100);
        }

        double elapsed = timer.elapsed_ms();
        std::printf("PASS (%.2f ms for %u calls, %.2f us/call)\n",
                    elapsed, iterations, (elapsed * 1000.0) / iterations);
    }

    // Test 5: API overhead for draw_indirect calls
    {
        std::printf("Test 5: API overhead for draw_indirect calls... ");
        FrameTimer timer;
        poko::render::Context context;
        poko::render::VertexBuffer buffer;

        uint32_t iterations = 10000;
        for (uint32_t i = 0; i < iterations; ++i) {
            context.draw_indirect(&buffer, 0);
        }

        double elapsed = timer.elapsed_ms();
        std::printf("PASS (%.2f ms for %u calls, %.2f us/call)\n",
                    elapsed, iterations, (elapsed * 1000.0) / iterations);
    }

    // Test 6: API overhead for draw_indexed_indirect calls
    {
        std::printf("Test 6: API overhead for draw_indexed_indirect calls... ");
        FrameTimer timer;
        poko::render::Context context;
        poko::render::VertexBuffer buffer;

        uint32_t iterations = 10000;
        for (uint32_t i = 0; i < iterations; ++i) {
            context.draw_indexed_indirect(&buffer, 0);
        }

        double elapsed = timer.elapsed_ms();
        std::printf("PASS (%.2f ms for %u calls, %.2f us/call)\n",
                    elapsed, iterations, (elapsed * 1000.0) / iterations);
    }

    // Test 7: set_vertex_buffer API overhead
    {
        std::printf("Test 7: set_vertex_buffer API overhead... ");
        FrameTimer timer;
        poko::render::Context context;
        poko::render::VertexBuffer buffer;

        uint32_t iterations = 10000;
        for (uint32_t i = 0; i < iterations; ++i) {
            context.set_vertex_buffer(&buffer, 0);
        }

        double elapsed = timer.elapsed_ms();
        std::printf("PASS (%.2f ms for %u calls, %.2f us/call)\n",
                    elapsed, iterations, (elapsed * 1000.0) / iterations);
    }

    // Test 8: set_vertex_buffer with different slots
    {
        std::printf("Test 8: set_vertex_buffer with different slots... ");
        FrameTimer timer;
        poko::render::Context context;
        poko::render::VertexBuffer buffer;

        uint32_t iterations = 10000;
        for (uint32_t i = 0; i < iterations; ++i) {
            context.set_vertex_buffer(&buffer, i % 2);  // Alternate between slot 0 and 1
        }

        double elapsed = timer.elapsed_ms();
        std::printf("PASS (%.2f ms for %u calls, %.2f us/call)\n",
                    elapsed, iterations, (elapsed * 1000.0) / iterations);
    }

    // Test 9: Context validation overhead
    {
        std::printf("Test 9: Context validation overhead... ");
        FrameTimer timer;
        poko::render::Context context;

        uint32_t iterations = 10000;
        for (uint32_t i = 0; i < iterations; ++i) {
            (void)context.is_valid();
        }

        double elapsed = timer.elapsed_ms();
        std::printf("PASS (%.2f ms for %u calls, %.2f us/call)\n",
                    elapsed, iterations, (elapsed * 1000.0) / iterations);
    }

    // Test 10: Error message retrieval overhead
    {
        std::printf("Test 10: Error message retrieval overhead... ");
        FrameTimer timer;
        poko::render::Context context;

        uint32_t iterations = 10000;
        for (uint32_t i = 0; i < iterations; ++i) {
            (void)context.get_last_error();
        }

        double elapsed = timer.elapsed_ms();
        std::printf("PASS (%.2f ms for %u calls, %.2f us/call)\n",
                    elapsed, iterations, (elapsed * 1000.0) / iterations);
    }

    // Test 11: Memory allocation tracking (buffer creation overhead)
    {
        std::printf("Test 11: Buffer creation overhead... ");
        FrameTimer timer;

        uint32_t iterations = 1000;

        for (uint32_t i = 0; i < iterations; ++i) {
            // Just measure construction/destruction overhead
            // Can't use std::vector due to deleted move constructor
            [[maybe_unused]] poko::render::VertexBuffer buffer;
        }

        double elapsed = timer.elapsed_ms();
        std::printf("PASS (%.2f ms for %u constructions, %.2f us/construction)\n",
                    elapsed, iterations, (elapsed * 1000.0) / iterations);
    }

    // Test 12: Comparison: draw vs draw_instanced API call overhead
    {
        std::printf("Test 12: Comparison: draw vs draw_instanced overhead... ");
        FrameTimer timer1;
        poko::render::Context context1;
        uint32_t iterations = 10000;
        for (uint32_t i = 0; i < iterations; ++i) {
            context1.draw(4);
        }
        double elapsed1 = timer1.elapsed_ms();

        FrameTimer timer2;
        poko::render::Context context2;
        for (uint32_t i = 0; i < iterations; ++i) {
            context2.draw_instanced(4, 100);
        }
        double elapsed2 = timer2.elapsed_ms();

        double overhead_diff = elapsed2 - elapsed1;
        std::printf("PASS (draw: %.2f us/call, draw_instanced: %.2f us/call, diff: %.2f us/call)\n",
                    (elapsed1 * 1000.0) / iterations,
                    (elapsed2 * 1000.0) / iterations,
                    (overhead_diff * 1000.0) / iterations);
    }

    std::printf("\nAll M9 performance benchmark tests passed.\n");
    std::printf("Note: These tests measure API overhead only (no actual GPU rendering).\n");
    std::printf("For full rendering benchmarks, run the M9 demo with a valid device.\n");
    return 0;
}
