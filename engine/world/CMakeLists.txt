# =============================================================================
# Poko World Module
#
# Provides scene graph, ECS storage, object handles, hierarchy, transform
# propagation, and serialization boundaries for the PokoX engine.
# =============================================================================

cmake_minimum_required(VERSION 3.24)

project(poko_world
    VERSION 0.1.0
    DESCRIPTION "PokoX world and scene graph module"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# -----------------------------------------------------------------------------
# Public API headers
# -----------------------------------------------------------------------------
set(POKO_WORLD_PUBLIC_HEADERS
    public/poko/world/world.h
    public/poko/world/scene.h
    public/poko/world/entity.h
    public/poko/world/transform.h
    public/poko/world/component.h
    public/poko/world/camera.h
    public/poko/world/collision.h
    public/poko/world/asset.h
    public/poko/world/character.h
    public/poko/world/model.h
    public/poko/world/skinned_model.h
)

# -----------------------------------------------------------------------------
# Source files organized by component
# -----------------------------------------------------------------------------
set(POKO_WORLD_SOURCES
    src/world/world.cpp
    src/world/world_impl.cpp
    src/scene/scene.cpp
    src/scene/scene_impl.cpp
    src/entity/entity.cpp
    src/entity/entity_impl.cpp
    src/transform/transform.cpp
    src/transform/transform_impl.cpp
    src/component/component.cpp
    src/camera/camera.cpp
    src/collision/collision.cpp
    src/asset/asset.cpp
    src/character/character.cpp
    src/model/model.cpp
    src/skinned_model/skinned_model.cpp
)

# -----------------------------------------------------------------------------
# Create the world library
# -----------------------------------------------------------------------------
add_library(poko_world STATIC
    ${POKO_WORLD_PUBLIC_HEADERS}
    ${POKO_WORLD_SOURCES}
)

target_include_directories(poko_world
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/public>
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

# Link core and math modules
target_link_libraries(poko_world
    PRIVATE
        poko_core
        poko_math
        poko_renderer
)

# Set warning level per target
if(MSVC)
    target_compile_options(poko_world PRIVATE /W4)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(poko_world PRIVATE -Wall -Wextra -Wpedantic)
endif()

# -----------------------------------------------------------------------------
# Install rules
# -----------------------------------------------------------------------------
install(TARGETS poko_world
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES ${POKO_WORLD_PUBLIC_HEADERS}
    DESTINATION include/poko/world
)

# -----------------------------------------------------------------------------
# Tests
# -----------------------------------------------------------------------------
add_executable(test_world
    ${CMAKE_CURRENT_SOURCE_DIR}/test_world.cpp
)

target_link_libraries(test_world
    PRIVATE
        poko_world
        poko_math
)

target_include_directories(test_world
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/engine/math/public>
)

add_test(NAME world.tests
    COMMAND test_world
)
