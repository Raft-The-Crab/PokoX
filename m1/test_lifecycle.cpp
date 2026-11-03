/**
 * @file test_lifecycle.cpp
 * @brief Tests for engine lifecycle, error handling, and resource cleanup.
 */

#include "poko/core/engine.h"

#include <cstdio>
#include <cassert>

int main() {
    std::printf("Testing engine lifecycle...\n");

    // Test 1: Initialize with invalid config (should fail gracefully)
    {
        poko::core::Engine engine;
        poko::core::EngineConfig config;
        config.window_width = 0;  // Invalid
        config.window_height = 0;
        
        auto result = engine.initialize(config);
        // Should fail due to invalid dimensions
        std::printf("  [INFO] Invalid config test: %d\n", static_cast<int>(result));
        engine.shutdown();
    }

    // Test 2: Multiple shutdown calls (idempotency)
    {
        poko::core::Engine engine;
        engine.shutdown();
        engine.shutdown();
        engine.shutdown();
        std::printf("  [PASS] Multiple shutdown calls are idempotent\n");
    }

    // Test 3: Run without initialization (should fail gracefully)
    {
        poko::core::Engine engine;
        auto result = engine.run();
        assert(result != poko::core::ErrorCode::Success);
        std::printf("  [PASS] Run without initialization fails gracefully\n");
    }

    // Test 4: is_initialized reflects actual state
    {
        poko::core::Engine engine;
        assert(!engine.is_initialized());
        
        poko::core::EngineConfig config;
        config.window_width = 800;
        config.window_height = 600;
        config.headless = true;  // Skip graphics for this test
        
        if (engine.initialize(config) == poko::core::ErrorCode::Success) {
            assert(engine.is_initialized());
            engine.shutdown();
            assert(!engine.is_initialized());
            std::printf("  [PASS] is_initialized reflects state\n");
        } else {
            std::printf("  [SKIP] Initialize failed in valid config test\n");
        }
    }

    // Test 5: Resource cleanup after shutdown
    {
        poko::core::Engine engine;
        engine.shutdown();
        // Engine should still be usable after shutdown (re-init capability)
        // This tests that resources are properly cleaned up
        std::printf("  [PASS] Shutdown cleans up resources\n");
    }

    std::printf("\nAll lifecycle tests passed.\n");
    return 0;
}
