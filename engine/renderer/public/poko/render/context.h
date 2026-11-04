/**
 * @file context.h
 * @brief Poko-owned command context abstraction.
 *
 * This header contains only Poko types. The Diligent Core implementation lives
 * behind the Pimpl pattern in backend/diligent/. Consumers never need to include
 * Diligent headers.
 *
 * M9 Scope: Adds instanced rendering and indirect drawing for efficient batched
 * geometry and GPU-driven draw calls.
 *
 * Usage example (instanced rendering):
 * @code
 * // Create a single quad mesh
 * VertexBuffer quad_vb;
 * // ... initialize quad_vb with 4 vertices ...
 *
 * // Create instance data (positions for 1000 quads)
 * VertexBuffer instance_vb;
 * // ... initialize instance_vb with 1000 instance positions ...
 *
 * // Bind both vertex buffers (stride for instance data must be set in vertex layout)
 * context.set_vertex_buffer(&quad_vb, 0);  // Slot 0: vertex data
 * context.set_vertex_buffer(&instance_vb, 1);  // Slot 1: instance data
 *
 * // Draw 1000 instances of the quad
 * context.draw_instanced(4, 1000);  // 4 vertices per instance, 1000 instances
 * @endcode
 *
 * Usage example (indirect drawing):
 * @code
 * // Create indirect draw arguments buffer
 * struct DrawIndirectArgs {
 *     uint32_t vertex_count;
 *     uint32_t instance_count;
 *     uint32_t first_vertex;
 *     uint32_t first_instance;
 * };
 *
 * VertexBuffer indirect_buffer;
 * BufferDesc desc;
 * desc.data = draw_args;
 * desc.size = sizeof(DrawIndirectArgs);
 * desc.stride = sizeof(DrawIndirectArgs);
 * indirect_buffer.initialize(device, desc);
 *
 * // Draw using GPU-driven arguments
 * // Note: Indirect drawing requires backend-specific implementation
 * // Currently returns error: "Indirect drawing not yet fully implemented"
 * context.draw_indirect(&indirect_buffer, 0);
 * @endcode
 *
 * Production Status:
 * - Instanced rendering: Fully implemented and tested
 * - Indirect drawing: API validation complete, backend implementation deferred
 *   (GPU-driven draw calls require additional buffer binding logic)
 */

#ifndef POKO_RENDER_CONTEXT_H
#define POKO_RENDER_CONTEXT_H

#include "device.h"
#include "swapchain.h"
#include "pipeline.h"
#include "vertex_buffer.h"
#include "index_buffer.h"
#include "texture.h"
#include "material.h"
#include "constant_buffer.h"
#include "framebuffer.h"
#include <cstdint>

namespace poko::render {

/**
 * @brief Viewport description.
 */
struct Viewport {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
    float min_depth = 0.0f;
    float max_depth = 1.0f;
};

/**
 * @brief Clear color (RGBA, linear or sRGB depending on format).
 */
struct ClearColor {
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 1.0f;
};

/**
 * @brief Scissor rectangle description.
 */
struct ScissorRect {
    int32_t left = 0;
    int32_t top = 0;
    int32_t right = 0;
    int32_t bottom = 0;
};

/**
 * @brief Opaque handle to a command context.
 *
 * The implementation is private; consumers cannot depend on its layout.
 */
class Context {
public:
    Context();
    ~Context();

    // Non-copyable, non-movable (owns backend resources)
    Context(const Context&) = delete;
    Context& operator=(const Context&) = delete;
    Context(Context&&) = delete;
    Context& operator=(Context&&) = delete;

    /**
     * @brief Sets the viewport.
     *
     * @param viewport Viewport description.
     *
     * Error conditions (call get_last_error() for details):
     * - Context is invalid
     * - width or height <= 0
     * - min_depth or max_depth not in [0, 1]
     * - min_depth >= max_depth
     */
    void set_viewport(const Viewport& viewport);

    /**
     * @brief Sets the scissor rectangle.
     *
     * Scissor testing must be enabled in the pipeline rasterizer state.
     *
     * @param scissor Scissor rectangle description.
     *
     * Error conditions (call get_last_error() for details):
     * - Context is invalid
     * - left or top < 0
     * - right <= left or bottom <= top
     */
    void set_scissor_rect(const ScissorRect& scissor);

    /**
     * @brief Clears the current render target.
     *
     * @param color Clear color.
     *
     * Error conditions (call get_last_error() for details):
     * - Context is invalid
     * - No swap chain bound
     * - Failed to get back buffer RTV
     */
    void clear_render_target(const ClearColor& color);

    /**
     * @brief Clears the current depth buffer.
     *
     * @param depth Depth value to clear to (default 1.0).
     * @param stencil Stencil value to clear to (default 0).
     *
     * Error conditions (call get_last_error() for details):
     * - Context is invalid
     * - No swap chain bound
     */
    void clear_depth(float depth = 1.0f, uint8_t stencil = 0);

    /**
     * @brief Sets the graphics pipeline state.
     *
     * @param pipeline The pipeline state. Must be valid and compatible with this context's device.
     * @return true on success, false on failure.
     *
     * Error conditions (call get_last_error() for details):
     * - Context is invalid
     * - Pipeline is invalid
     * - Pipeline created from different device
     */
    bool set_pipeline_state(PipelineState* pipeline);

    /**
     * @brief Sets the render target from a swap chain.
     *
     * This must be called before clearing or drawing in the OpenGL backend.
     *
     * @param swap_chain The swap chain. Must be valid.
     * @return true on success, false on failure.
     *
     * Error conditions (call get_last_error() for details):
     * - Context is invalid
     * - Swap chain is invalid
     * - Failed to get swap chain implementation
     * - Failed to get back buffer RTV
     */
    bool set_render_target(SwapChain* swap_chain);

    /**
     * @brief Sets the render target from a framebuffer (render-to-texture).
     *
     * This enables render-to-texture functionality. When a framebuffer is bound,
     * all rendering operations write to the framebuffer's color attachments instead
     * of the swap chain. The framebuffer must have at least one color attachment.
     *
     * To switch back to rendering to the screen, call set_render_target(SwapChain*).
     *
     * @param framebuffer The framebuffer. Must be valid.
     * @return true on success, false on failure.
     *
     * Error conditions (call get_last_error() for details):
     * - Context is invalid
     * - Framebuffer is invalid
     * - Framebuffer created from different device
     * - Failed to get framebuffer implementation
     *
     * Note: The framebuffer's dimensions define the viewport for subsequent draws.
     * Call set_viewport() after binding a framebuffer to ensure correct rendering.
     */
    bool set_render_target(Framebuffer* framebuffer);

    /**
     * @brief Binds a vertex buffer for drawing.
     *
     * @param buffer The vertex buffer. Must be valid.
     * @param slot Input slot (0 for vertex data, 1+ for instance data). Default 0.
     * @return true on success, false on failure.
     *
     * Error conditions (call get_last_error() for details):
     * - Context is invalid
     * - Vertex buffer is invalid
     * - Vertex buffer created from different device
     * - Slot exceeds maximum (16)
     */
    bool set_vertex_buffer(VertexBuffer* buffer, uint32_t slot = 0);

    /**
     * @brief Binds an index buffer for indexed drawing.
     *
     * @param buffer The index buffer. Must be valid.
     * @return true on success, false on failure.
     *
     * Error conditions (call get_last_error() for details):
     * - Context is invalid
     * - Index buffer is invalid
     * - Index buffer created from different device
     */
    bool set_index_buffer(IndexBuffer* buffer);

    /**
     * @brief Draws non-indexed primitives.
     *
     * @param vertex_count Number of vertices to draw.
     * @return true on success, false on failure.
     *
     * Error conditions (call get_last_error() for details):
     * - Context is invalid
     * - vertex_count is 0
     */
    bool draw(uint32_t vertex_count);

    /**
     * @brief Draws indexed primitives.
     *
     * @param index_count Number of indices to draw.
     * @param start_index Offset into the index buffer.
     * @return true on success, false on failure.
     *
     * Error conditions (call get_last_error() for details):
     * - Context is invalid
     * - index_count is 0
     */
    bool draw_indexed(uint32_t index_count, uint32_t start_index = 0);

    /**
     * @brief Draws non-indexed instanced primitives (M9).
     *
     * Instanced rendering allows drawing multiple instances of the same geometry
     * with different per-instance data (e.g., positions, colors, transforms).
     * The instance data is provided through a separate vertex buffer bound to
     * a different input slot.
     *
     * @param vertex_count Number of vertices per instance.
     * @param instance_count Number of instances to draw.
     * @param start_instance Offset for first instance (default 0).
     * @return true on success, false on failure.
     *
     * Error conditions (call get_last_error() for details):
     * - Context is invalid
     * - vertex_count is 0
     * - instance_count is 0
     *
     * Note: The vertex layout must include instance attributes with appropriate
     * input slot numbers. Instance data buffer must be bound to the corresponding slot.
     */
    bool draw_instanced(uint32_t vertex_count, uint32_t instance_count, uint32_t start_instance = 0);

    /**
     * @brief Draws indexed instanced primitives (M9).
     *
     * Combines indexed drawing with instanced rendering for efficient batched
     * geometry rendering with per-instance variation.
     *
     * @param index_count Number of indices per instance.
     * @param instance_count Number of instances to draw.
     * @param start_index Offset into the index buffer (default 0).
     * @param start_instance Offset for first instance (default 0).
     * @return true on success, false on failure.
     *
     * Error conditions (call get_last_error() for details):
     * - Context is invalid
     * - index_count is 0
     * - instance_count is 0
     */
    bool draw_indexed_instanced(uint32_t index_count, uint32_t instance_count,
                                 uint32_t start_index = 0, uint32_t start_instance = 0);

    /**
     * @brief Draws non-indexed primitives with GPU-driven arguments (M9).
     *
     * Indirect drawing allows the GPU to determine draw parameters, enabling
     * GPU-driven rendering pipelines and dynamic draw call batching.
     *
     * The indirect buffer must contain DrawIndirectArgs structure:
     *   - vertex_count (uint32_t): Number of vertices to draw
     *   - instance_count (uint32_t): Number of instances to draw
     *   - first_vertex (uint32_t): Offset into vertex buffer
     *   - first_instance (uint32_t): Offset for first instance
     *
     * @param indirect_buffer Buffer containing draw arguments.
     * @param offset Byte offset into the indirect buffer (default 0).
     * @return true on success, false on failure.
     *
     * Error conditions (call get_last_error() for details):
     * - Context is invalid
     * - Indirect buffer is null or invalid
     * - Indirect buffer created from different device
     * - Offset is not aligned to 4 bytes
     */
    bool draw_indirect(VertexBuffer* indirect_buffer, uint32_t offset = 0);

    /**
     * @brief Draws indexed primitives with GPU-driven arguments (M9).
     *
     * Combines indexed drawing with indirect arguments for GPU-driven
     * indexed rendering.
     *
     * The indirect buffer must contain DrawIndexedIndirectArgs structure:
     *   - index_count (uint32_t): Number of indices to draw
     *   - instance_count (uint32_t): Number of instances to draw
     *   - first_index (uint32_t): Offset into index buffer
     *   - vertex_offset (int32_t): Offset added to vertex indices
     *   - first_instance (uint32_t): Offset for first instance
     *
     * @param indirect_buffer Buffer containing draw arguments.
     * @param offset Byte offset into the indirect buffer (default 0).
     * @return true on success, false on failure.
     *
     * Error conditions (call get_last_error() for details):
     * - Context is invalid
     * - Indirect buffer is null or invalid
     * - Indirect buffer created from different device
     * - Offset is not aligned to 4 bytes
     */
    bool draw_indexed_indirect(VertexBuffer* indirect_buffer, uint32_t offset = 0);

    /**
     * @brief Binds a texture to a shader resource slot.
     *
     * @param texture The texture. Must be valid.
     * @param slot Shader resource slot (0-based).
     * @return true on success, false on failure.
     */
    bool set_texture(Texture* texture, uint32_t slot = 0);

    /**
     * @brief Binds a material (groups texture-slot bindings).
     *
     * @param material The material. Must be valid.
     * @return true on success, false on failure.
     *
     * Error conditions (call get_last_error() for details):
     * - Context is invalid
     * - Material is invalid
     * - Material created from different device
     */
    bool set_material(Material* material);

    /**
     * @brief Flushes pending commands.
     */
    void flush();

    /**
     * @brief Returns true if the context is valid.
     */
    bool is_valid() const;

    /**
     * @brief Returns the backend type for this context.
     */
    BackendType backend_type() const;

    /**
     * @brief Returns the last error message from context operations.
     */
    const char* get_last_error() const;

    /**
     * @brief Initializes the context from a device and swap chain.
     *
     * @param device The graphics device.
     * @param swap_chain The swap chain.
     * @return true on success.
     */
    bool initialize(Device* device, SwapChain* swap_chain);

private:
    struct Impl;
    Impl* impl_;
    
    // Allow backend implementation to access private impl_
    friend Context::Impl* get_context_impl(Context*);
};

} // namespace poko::render

#endif // POKO_RENDER_CONTEXT_H
