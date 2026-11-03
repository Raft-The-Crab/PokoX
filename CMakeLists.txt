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

option(POKO_BUILD_M0        "Build the M0 technical-feasibility applications" ON)
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
# Tests
# -----------------------------------------------------------------------------
if(BUILD_TESTING)
    poko_add_component_if_present("tests")
endif()

# -----------------------------------------------------------------------------
# Configure-time summary: what this build actually contains.
# -----------------------------------------------------------------------------
poko_print_build_summary()
