# =============================================================================
# PokoX — root build configuration
#
# Design goals (see docs/03_ENGINE_ARCHITECTURE.md, docs/26_BUILD_ENVIRONMENT_AND_CMAKE.md):
#   * every module is a self-contained target with a public/private boundary;
#   * third-party dependencies are pinned and their include paths never leak
#     globally;
#   * the tree configures and builds on a machine where most engine modules do
#     not exist yet, so the build reflects reality instead of an aspiration.
# =============================================================================
cmake_minimum_required(VERSION 3.24)

project(PokoX
    VERSION 0.1.0
    DESCRIPTION "PokoX game platform: shared C++ engine, applications and services"
    LANGUAGES C CXX)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/modules")
include(PokoHelpers)
include(PokoDependencies)

# -----------------------------------------------------------------------------
# Baseline language settings
# -----------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
    message(STATUS "poko: CMAKE_BUILD_TYPE not set, defaulting to Debug")
endif()

# All executables land in one directory so runtime DLL bundling and manual
# launches behave identically for every target.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# -----------------------------------------------------------------------------
# Options
# -----------------------------------------------------------------------------
option(POKO_BUILD_CLIENT    "Build the PokoX client host"           ON)
option(POKO_BUILD_STUDIO    "Build the Poko Studio host"            ON)
option(POKO_BUILD_MOD       "Build the Poko Mod host"               ON)
option(POKO_BUILD_AUTHORITY "Build the headless game authority"     ON)

option(POKO_BUILD_M0        "Build the M0 technical-feasibility applications (reference, disabled by default)" OFF)
option(POKO_BUILD_M1        "Build the M1 engine foundation demo" OFF)
option(POKO_BUILD_M2        "Build the M2 minimal scene rendering demo" OFF)
option(POKO_BUILD_M3        "Build the M3 vertex buffer and mesh rendering demo" OFF)
option(POKO_BUILD_M4        "Build the M4 index buffer and indexed rendering demo" OFF)
option(POKO_BUILD_M5        "Build the M5 texture creation demo" OFF)
option(POKO_BUILD_M6        "Build the M6 material and SRB demo" OFF)
option(POKO_BUILD_M7        "Build the M7 constant buffer demo" OFF)
option(POKO_BUILD_M8        "Build the M8 render passes and framebuffers demo" ON)
option(POKO_BUILD_M9        "Build the M9 advanced drawing and performance benchmarks demo" ON)
option(POKO_WITH_DILIGENT   "Build the Diligent Core graphics backend"        ON)

include(CTest)  # defines BUILD_TESTING, registers CTest for `ctest`

# -----------------------------------------------------------------------------
# Third-party dependencies (pinned; see cmake/modules/PokoDependencies.cmake)
# -----------------------------------------------------------------------------
poko_find_sdl2()
poko_fetch_diligent_core()

# -----------------------------------------------------------------------------
# Engine modules
#
# Modules are added only when their CMakeLists.txt exists, so an unimplemented
# module is reported once at configure time instead of breaking the build. When
# a module lands in M1 it is picked up automatically, with no root-file edit.
# -----------------------------------------------------------------------------
set(POKO_ENGINE_MODULES
    core math platform input world assets renderer
    physics animation audio ui_runtime mute network diagnostics)

foreach(poko_module IN LISTS POKO_ENGINE_MODULES)
    poko_add_component_if_present("engine/${poko_module}")
endforeach()

# -----------------------------------------------------------------------------
# Application hosts
# -----------------------------------------------------------------------------
if(POKO_BUILD_CLIENT)
    poko_add_component_if_present("applications/pokox_client_host")
endif()

if(POKO_BUILD_STUDIO)
    poko_add_component_if_present("applications/poko_studio_host")
endif()

if(POKO_BUILD_MOD)
    poko_add_component_if_present("applications/poko_mod_host")
    poko_add_component_if_present("applications/poko_review_host")
endif()

if(POKO_BUILD_AUTHORITY)
    poko_add_component_if_present("applications/poko_authority_host")
endif()

# -----------------------------------------------------------------------------
# M0 technical feasibility applications
# -----------------------------------------------------------------------------
if(POKO_BUILD_M0)
    add_subdirectory(m0)
endif()

# -----------------------------------------------------------------------------
# M1 engine foundation
# -----------------------------------------------------------------------------
if(POKO_BUILD_M1)
    add_subdirectory(m1)
endif()

# -----------------------------------------------------------------------------
# M2 minimal scene rendering
# -----------------------------------------------------------------------------
if(POKO_BUILD_M2)
    add_subdirectory(m2)
endif()

# -----------------------------------------------------------------------------
# M3 vertex buffers and mesh rendering
# -----------------------------------------------------------------------------
if(POKO_BUILD_M3)
    add_subdirectory(m3)
endif()

# -----------------------------------------------------------------------------
# M4 index buffers and indexed rendering
# -----------------------------------------------------------------------------
if(POKO_BUILD_M4)
    add_subdirectory(m4)
endif()

# -----------------------------------------------------------------------------
# M5 textures and texture sampling
# -----------------------------------------------------------------------------
if(POKO_BUILD_M5)
    add_subdirectory(m5)
endif()

# -----------------------------------------------------------------------------
# M6 materials and shader resource bindings
# -----------------------------------------------------------------------------
if(POKO_BUILD_M6)
    add_subdirectory(m6)
endif()

# -----------------------------------------------------------------------------
# M7 constant buffers
# -----------------------------------------------------------------------------
if(POKO_BUILD_M7)
    add_subdirectory(m7)
endif()

# -----------------------------------------------------------------------------
# M8 render passes and framebuffers
# -----------------------------------------------------------------------------
if(POKO_BUILD_M8)
    add_subdirectory(m8)
endif()

# -----------------------------------------------------------------------------
# M9 advanced drawing and performance benchmarks
# -----------------------------------------------------------------------------
if(POKO_BUILD_M9)
    add_subdirectory(m9)
endif()

# -----------------------------------------------------------------------------
# Tests
# -----------------------------------------------------------------------------
if(BUILD_TESTING)
    poko_add_component_if_present("tests")
endif()

# -----------------------------------------------------------------------------
# Documentation
# -----------------------------------------------------------------------------
find_package(Doxygen)
if(DOXYGEN_FOUND)
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/docs)

    add_custom_target(doc
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_IN}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )
else()
    message(STATUS "Doxygen not found, documentation target will not be available")
endif()

# -----------------------------------------------------------------------------
# Configure-time summary: what this build actually contains.
# -----------------------------------------------------------------------------
poko_print_build_summary()
