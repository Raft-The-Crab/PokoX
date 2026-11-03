/**
 * @file test_public_headers.cpp
 * @brief Compile-only test: verifies public Poko headers do not require Diligent headers.
 *
 * This test should compile successfully with only the public Poko headers included.
 * If Diligent types leak into the public API, this will fail to compile.
 *
 * This is a compile-only test; the executable is never run.
 */

#include "poko/render/device.h"
#include "poko/render/swapchain.h"
#include "poko/render/context.h"
#include "poko/platform/window.h"
#include "poko/core/engine.h"

// Verify the types are accessible without Diligent headers
static_assert(sizeof(poko::render::Device) > 0, "Device size check");
static_assert(sizeof(poko::render::SwapChain) > 0, "SwapChain size check");
static_assert(sizeof(poko::render::Context) > 0, "Context size check");
static_assert(sizeof(poko::platform::Window) > 0, "Window size check");
static_assert(sizeof(poko::core::Engine) > 0, "Engine size check");

// Verify enums are accessible
static_assert(static_cast<int>(poko::render::BackendType::OpenGL) > 0, "BackendType check");
static_assert(static_cast<int>(poko::render::TextureFormat::RGBA8_UNORM) > 0, "TextureFormat check");

int main() {
    // This test is compile-only; the executable is never run.
    return 0;
}
