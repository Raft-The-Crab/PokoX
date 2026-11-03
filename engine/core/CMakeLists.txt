# =============================================================================
# Poko Core Module
#
# Provides engine initialization, lifecycle, and main loop.
# =============================================================================

cmake_minimum_required(VERSION 3.24)

project(poko_core
    VERSION 0.1.0
    DESCRIPTION "PokoX core engine module"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# -----------------------------------------------------------------------------
# Public API headers
# -----------------------------------------------------------------------------
set(POKO_CORE_PUBLIC_HEADERS
    public/poko/core/engine.h
)

# -----------------------------------------------------------------------------
# Source files organized by component
# -----------------------------------------------------------------------------
set(POKO_CORE_SOURCES
    src/initialization/engine.cpp
    src/initialization/engine_impl.cpp
)

# -----------------------------------------------------------------------------
# Create the core library
# -----------------------------------------------------------------------------
add_library(poko_core STATIC
    ${POKO_CORE_PUBLIC_HEADERS}
    ${POKO_CORE_SOURCES}
)

target_include_directories(poko_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/public>
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

# Link platform and renderer modules
target_link_libraries(poko_core
    PRIVATE
        poko_platform
        poko_renderer
        SDL2::SDL2
)

# Set warning level per target
if(MSVC)
    target_compile_options(poko_core PRIVATE /W4)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(poko_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

# -----------------------------------------------------------------------------
# Install rules
# -----------------------------------------------------------------------------
install(TARGETS poko_core
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES ${POKO_CORE_PUBLIC_HEADERS}
    DESTINATION include/poko/core
)
