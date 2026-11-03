# =============================================================================
# Poko Platform Module
#
# Provides OS abstraction for windows, events, and platform-specific resources.
# SDL2 implementation is hidden behind the Pimpl pattern.
# =============================================================================

cmake_minimum_required(VERSION 3.24)

project(poko_platform
    VERSION 0.1.0
    DESCRIPTION "PokoX platform abstraction layer"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# -----------------------------------------------------------------------------
# Public API headers (no SDL includes)
# -----------------------------------------------------------------------------
set(POKO_PLATFORM_PUBLIC_HEADERS
    public/poko/platform/window.h
)

# -----------------------------------------------------------------------------
# Source files organized by component
# -----------------------------------------------------------------------------
set(POKO_PLATFORM_SOURCES
    src/window/window.cpp
    src/window/window_impl.cpp
)

# -----------------------------------------------------------------------------
# Create the platform library
# -----------------------------------------------------------------------------
add_library(poko_platform STATIC
    ${POKO_PLATFORM_PUBLIC_HEADERS}
    ${POKO_PLATFORM_SOURCES}
)

target_include_directories(poko_platform
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/public>
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

# Link SDL2
target_link_libraries(poko_platform
    PRIVATE
        SDL2::SDL2
)

# Set warning level per target
if(MSVC)
    target_compile_options(poko_platform PRIVATE /W4)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(poko_platform PRIVATE -Wall -Wextra -Wpedantic)
endif()

# -----------------------------------------------------------------------------
# Install rules
# -----------------------------------------------------------------------------
install(TARGETS poko_platform
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES ${POKO_PLATFORM_PUBLIC_HEADERS}
    DESTINATION include/poko/platform
)
