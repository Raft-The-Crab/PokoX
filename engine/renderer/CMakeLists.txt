# =============================================================================
# Poko Renderer Module
#
# Provides a Poko-owned abstraction over graphics backends (Diligent Core).
# Public headers contain only Poko types; Diligent implementation is hidden
# behind the Pimpl pattern in backend/diligent/.
# =============================================================================

cmake_minimum_required(VERSION 3.24)

project(poko_renderer
    VERSION 0.1.0
    DESCRIPTION "PokoX renderer abstraction layer"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# -----------------------------------------------------------------------------
# Public API headers (no Diligent includes)
# -----------------------------------------------------------------------------
set(POKO_RENDERER_PUBLIC_HEADERS
    public/poko/render/device.h
    public/poko/render/swapchain.h
    public/poko/render/context.h
    public/poko/render/pipeline.h
    public/poko/render/vertex_buffer.h
    public/poko/render/index_buffer.h
    public/poko/render/texture.h
    public/poko/render/material.h
    public/poko/render/constant_buffer.h
    public/poko/render/framebuffer.h
    public/poko/render/mesh.h
    public/poko/render/obj_loader.h
    public/poko/render/texture_loader.h
    public/poko/render/gltf_loader.h
    public/poko/render/skinned_model.h
)

# -----------------------------------------------------------------------------
# Source files organized by component
# -----------------------------------------------------------------------------
set(POKO_RENDERER_SOURCES
    # Device
    src/device/device.cpp

    # SwapChain
    src/swapchain/swapchain.cpp

    # Context
    src/context/context.cpp

    # Pipeline
    src/pipeline/pipeline.cpp

    # VertexBuffer
    src/vertex_buffer/vertex_buffer.cpp

    # IndexBuffer
    src/index_buffer/index_buffer.cpp

    # Texture
    src/texture/texture.cpp

    # Material
    src/material/material.cpp

    # ConstantBuffer
    src/constant_buffer/constant_buffer.cpp

    # Framebuffer
    src/framebuffer/framebuffer.cpp

    # Mesh
    src/mesh/mesh.cpp

    # OBJ Loader
    src/obj_loader/obj_loader.cpp

    # Texture Loader
    src/texture_loader/texture_loader.cpp

    # glTF Loader (M13)
    src/gltf_loader/gltf_loader.cpp

    # Skinned Model (M13)
    src/skinned_model/skinned_model.cpp

    # Backend: Diligent implementation
    src/backend/diligent/device_impl.cpp
    src/backend/diligent/swapchain_impl.cpp
    src/backend/diligent/context_impl.cpp
    src/backend/diligent/pipeline_impl.cpp
    src/backend/diligent/vertex_buffer_impl.cpp
    src/backend/diligent/index_buffer_impl.cpp
    src/backend/diligent/texture_impl.cpp
    src/backend/diligent/material_impl.cpp
    src/backend/diligent/constant_buffer_impl.cpp
    src/backend/diligent/framebuffer_impl.cpp
)

# -----------------------------------------------------------------------------
# Create the renderer library
# -----------------------------------------------------------------------------
add_library(poko_renderer STATIC
    ${POKO_RENDERER_PUBLIC_HEADERS}
    ${POKO_RENDERER_SOURCES}
)

target_include_directories(poko_renderer
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/public>
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

# Link Diligent Core (private implementation detail) and math (public:
# glTF/skinned-model headers expose poko::math types)
if(POKO_WITH_DILIGENT)
    target_link_libraries(poko_renderer
        PRIVATE
            Diligent-GraphicsEngineOpenGL-static
            Diligent-Common
            Diligent-Win32Platform
            Diligent-GraphicsAccessories
            SDL2::SDL2
    )
endif()

target_link_libraries(poko_renderer
    PUBLIC
        poko_math
)

# Set warning level per target
if(MSVC)
    target_compile_options(poko_renderer PRIVATE /W4)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(poko_renderer PRIVATE -Wall -Wextra -Wpedantic)
endif()

# -----------------------------------------------------------------------------
# Install rules
# -----------------------------------------------------------------------------
install(TARGETS poko_renderer
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES ${POKO_RENDERER_PUBLIC_HEADERS}
    DESTINATION include/poko/render
)

# -----------------------------------------------------------------------------
# Tests
# -----------------------------------------------------------------------------
if(POKO_WITH_TESTS)
    add_executable(test_mesh
        tests/test_mesh.cpp
    )
    target_link_libraries(test_mesh
        PRIVATE
            poko_renderer
    )
    add_test(NAME test_mesh COMMAND test_mesh)

    add_executable(test_texture_loader
        tests/test_texture_loader.cpp
    )
    target_link_libraries(test_texture_loader
        PRIVATE
            poko_renderer
    )
    add_test(NAME test_texture_loader COMMAND test_texture_loader)

    add_executable(test_gltf_loader
        tests/test_gltf_loader.cpp
    )
    target_link_libraries(test_gltf_loader
        PRIVATE
            poko_renderer
    )
    add_test(NAME test_gltf_loader COMMAND test_gltf_loader)
endif()
