cmake_minimum_required(VERSION 3.20)
project(PokoX VERSION 0.1.0 LANGUAGES CXX)

# Build options
option(POKO_BUILD_CLIENT "Build PokoX Client" ON)
option(POKO_BUILD_STUDIO "Build Poko Studio" ON)
option(POKO_BUILD_MOD "Build Poko Mod" ON)
option(POKO_BUILD_AUTHORITY "Build headless authority" ON)
option(POKO_BUILD_TESTS "Build tests" ON)
option(POKO_ENABLE_FUZZING "Enable fuzz tests" OFF)

# C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/engine)

# Third-party dependencies
# TODO: Add DiligentCore and SDL2 as submodules or FetchContent

# Engine modules
add_subdirectory(engine/core)
add_subdirectory(engine/math)
add_subdirectory(engine/platform)
add_subdirectory(engine/input)
add_subdirectory(engine/world)
add_subdirectory(engine/assets)
add_subdirectory(engine/renderer)
add_subdirectory(engine/physics)
add_subdirectory(engine/animation)
add_subdirectory(engine/audio)
add_subdirectory(engine/ui_runtime)
add_subdirectory(engine/mute)
add_subdirectory(engine/network)
add_subdirectory(engine/diagnostics)

# Applications
if(POKO_BUILD_CLIENT)
    add_subdirectory(applications/pokox_client_host)
endif()

if(POKO_BUILD_STUDIO)
    add_subdirectory(applications/poko_studio_host)
endif()

if(POKO_BUILD_MOD)
    add_subdirectory(applications/poko_mod_host)
    add_subdirectory(applications/poko_review_host)
endif()

if(POKO_BUILD_AUTHORITY)
    add_subdirectory(applications/poko_authority_host)
endif()

# Tests
if(POKO_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
