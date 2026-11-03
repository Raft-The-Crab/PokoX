/**
 * @file context.h
 * @brief Poko-owned command context abstraction.
 *
 * This header contains only Poko types. The Diligent Core implementation lives
 * behind the Pimpl pattern in backend/diligent/. Consumers never need to include
 * Diligent headers.
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
     */
    void set_viewport(const Viewport& viewport);

    /**
     * @brief Clears the current render target.
     *
     * @param color Clear color.
     */
    void clear_render_target(const ClearColor& color);

    /**
     * @brief Clears the current depth buffer.
     *
     * @param depth Depth value to clear to (default 1.0).
     * @param stencil Stencil value to clear to (default 0).
     */
    void clear_depth(float depth = 1.0f, uint8_t stencil = 0);

    /**
     * @brief Sets the graphics pipeline state.
     *
     * @param pipeline The pipeline state. Must be valid and compatible with this context's device.
     * @return true on success, false on failure.
     */
    bool set_pipeline_state(PipelineState* pipeline);

    /**
     * @brief Sets the render target from a swap chain.
     *
     * This must be called before clearing or drawing in the OpenGL backend.
     *
     * @param swap_chain The swap chain. Must be valid.
     * @return true on success, false on failure.
     */
    bool set_render_target(SwapChain* swap_chain);

    /**
     * @brief Binds a vertex buffer for drawing.
     *
     * @param buffer The vertex buffer. Must be valid.
     * @return true on success, false on failure.
     */
    bool set_vertex_buffer(VertexBuffer* buffer);

    /**
     * @brief Binds an index buffer for indexed drawing.
     *
     * @param buffer The index buffer. Must be valid.
     * @return true on success, false on failure.
     */
    bool set_index_buffer(IndexBuffer* buffer);

    /**
     * @brief Draws non-indexed primitives.
     *
     * @param vertex_count Number of vertices to draw.
     * @return true on success, false on failure.
     */
    bool draw(uint32_t vertex_count);

    /**
     * @brief Draws indexed primitives.
     *
     * @param index_count Number of indices to draw.
     * @param start_index Offset into the index buffer.
     * @return true on success, false on failure.
     */
    bool draw_indexed(uint32_t index_count, uint32_t start_index = 0);

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
