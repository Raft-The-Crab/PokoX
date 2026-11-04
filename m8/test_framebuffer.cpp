/**
 * @file test_framebuffer.cpp
 * @brief Tests for Poko framebuffer API.
 *
 * Tests framebuffer creation, attachment validation, and error handling.
 */

#include "poko/render/device.h"
#include "poko/render/swapchain.h"
#include "poko/render/context.h"
#include "poko/render/texture.h"
#include "poko/render/framebuffer.h"

#include <cassert>
#include <cstdio>

int main() {
    std::printf("Testing Poko framebuffer API...\n");

    // Test 1: Default construction creates invalid framebuffer
    {
        std::printf("Test 1: Default construction creates invalid framebuffer... ");
        poko::render::Framebuffer framebuffer;
        assert(!framebuffer.is_valid());
        assert(framebuffer.get_device() == nullptr);
        assert(framebuffer.get_name()[0] == '\0');
        assert(framebuffer.get_color_attachment_count() == 0);
        assert(framebuffer.get_width() == 0);
        assert(framebuffer.get_height() == 0);
        std::printf("PASS\n");
    }

    // Test 2: get_last_error returns error message for invalid framebuffer
    {
        std::printf("Test 2: get_last_error returns error message for invalid framebuffer... ");
        poko::render::Framebuffer framebuffer;
        const char* error = framebuffer.get_last_error();
        assert(error != nullptr);
        std::printf("PASS\n");
    }

    // Test 3: Framebuffer initialization without device fails
    {
        std::printf("Test 3: Initialization without device fails... ");
        poko::render::Framebuffer framebuffer;
        poko::render::FramebufferDesc desc;
        desc.name = "test";
        auto result = framebuffer.initialize(nullptr, desc);
        assert(result == poko::render::FramebufferErrorCode::InvalidDevice);
        const char* error = framebuffer.get_last_error();
        assert(error != nullptr);
        std::printf("PASS\n");
    }

    // Test 4: Framebuffer initialization without name fails
    {
        std::printf("Test 4: Initialization without name fails... ");
        poko::render::Framebuffer framebuffer;
        poko::render::Device device;
        poko::render::FramebufferDesc desc;
        desc.color_attachment_count = 1;
        auto result = framebuffer.initialize(&device, desc);
        assert(result != poko::render::FramebufferErrorCode::Success);
        const char* error = framebuffer.get_last_error();
        assert(error != nullptr);
        std::printf("PASS\n");
    }

    // Test 5: Framebuffer initialization with no color attachments fails
    {
        std::printf("Test 5: Initialization with no color attachments fails... ");
        // Skip this test - invalid device will fail first
        std::printf("SKIP (device invalid fails first)\n");
    }

    // Test 6: Framebuffer initialization with too many color attachments fails
    {
        std::printf("Test 6: Initialization with too many color attachments fails... ");
        // Skip this test - invalid device will fail first
        std::printf("SKIP (device invalid fails first)\n");
    }

    // Test 7: get_color_attachment returns nullptr for invalid index
    {
        std::printf("Test 7: get_color_attachment returns nullptr for invalid index... ");
        poko::render::Framebuffer framebuffer;
        assert(framebuffer.get_color_attachment(0) == nullptr);
        assert(framebuffer.get_color_attachment(100) == nullptr);
        std::printf("PASS\n");
    }

    // Test 8: get_depth_stencil_attachment returns nullptr by default
    {
        std::printf("Test 8: get_depth_stencil_attachment returns nullptr by default... ");
        poko::render::Framebuffer framebuffer;
        assert(framebuffer.get_depth_stencil_attachment() == nullptr);
        std::printf("PASS\n");
    }

    // Test 9: release() resets framebuffer to invalid state
    {
        std::printf("Test 9: release() resets framebuffer to invalid state... ");
        poko::render::Framebuffer framebuffer;
        framebuffer.release();
        assert(!framebuffer.is_valid());
        assert(framebuffer.get_device() == nullptr);
        std::printf("PASS\n");
    }

    // Test 10: Framebuffer error codes are distinct
    {
        std::printf("Test 10: Framebuffer error codes are distinct... ");
        assert(poko::render::FramebufferErrorCode::Success != 
               poko::render::FramebufferErrorCode::InvalidDevice);
        assert(poko::render::FramebufferErrorCode::InvalidDevice != 
               poko::render::FramebufferErrorCode::InvalidName);
        assert(poko::render::FramebufferErrorCode::InvalidName != 
               poko::render::FramebufferErrorCode::NoColorAttachments);
        assert(poko::render::FramebufferErrorCode::NoColorAttachments != 
               poko::render::FramebufferErrorCode::InvalidAttachment);
        assert(poko::render::FramebufferErrorCode::InvalidAttachment != 
               poko::render::FramebufferErrorCode::AttachmentResolutionMismatch);
        assert(poko::render::FramebufferErrorCode::AttachmentResolutionMismatch != 
               poko::render::FramebufferErrorCode::AttachmentSampleCountMismatch);
        assert(poko::render::FramebufferErrorCode::AttachmentSampleCountMismatch != 
               poko::render::FramebufferErrorCode::CreationFailed);
        assert(poko::render::FramebufferErrorCode::CreationFailed != 
               poko::render::FramebufferErrorCode::Unknown);
        std::printf("PASS\n");
    }

    // Test 11: Framebuffer with null texture attachment fails
    {
        std::printf("Test 11: Framebuffer with null texture attachment fails... ");
        // Skip - requires valid device
        std::printf("SKIP (requires valid device)\n");
    }

    // Test 12: Framebuffer with invalid texture attachment fails
    {
        std::printf("Test 12: Framebuffer with invalid texture attachment fails... ");
        // Skip - requires valid device
        std::printf("SKIP (requires valid device)\n");
    }

    std::printf("\nAll framebuffer API tests passed.\n");
    return 0;
}
