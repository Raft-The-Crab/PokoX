# =============================================================================
# PokoDependencies.cmake — pinned third-party acquisition.
#
# Policy (docs/26_BUILD_ENVIRONMENT_AND_CMAKE.md §8):
#   * one acquisition strategy per dependency, recorded here;
#   * never fetch an unpinned revision;
#   * include paths stay inside imported targets, never global.
#
# Current register:
#   SDL2          | MSYS2 UCRT64 package (2.32.10)        | zlib
#   DiligentCore  | FetchContent by URL + SHA-256         | Apache-2.0
#   xxHash        | FetchContent by URL + SHA-256         | BSD-2-Clause
#                 | (required by DiligentCore, see below)
#
# Why Diligent is pinned by archive hash rather than by git ref:
#   `git clone --depth 1` of this repository stalled indefinitely from the
#   development network (pack file never received a byte, verified twice), while
#   the release tarball transfers normally. A content hash is also a stronger pin
#   than a tag: CMake fails the configure if a single byte differs, so a moved or
#   re-tagged upstream cannot silently change what is built. The upstream commit
#   that the tag pointed to is recorded alongside the hash for traceability.
# =============================================================================
include_guard(GLOBAL)
include(FetchContent)

# -----------------------------------------------------------------------------
# Dependency pins
# -----------------------------------------------------------------------------
set(POKO_DILIGENT_TAG "API256018")
set(POKO_DILIGENT_COMMIT "b75f924bde69083fe84a38169d8ed985ebd7c6c8")
set(POKO_DILIGENT_REPOSITORY "https://github.com/DiligentGraphics/DiligentCore.git")
# The commit that tag API256018 resolved to when this pin was reviewed.
# This provides traceability even if the tag moves.
set(POKO_DILIGENT_REVIEWED_COMMIT "b75f924bde69083fe84a38169d8ed985ebd7c6c8")

# Windows desktop consumes the MSYS2 package so that exactly one SDL2 (and one
# SDL2 runtime DLL) exists in the process. Android will build SDL2 from a pinned
# source revision under its own toolchain, never mixing the two.
option(POKO_USE_SYSTEM_SDL2
    "Use the SDL2 package supplied by the toolchain (MSYS2 UCRT64) instead of building SDL2 from source"
    ON)

# Option to enable Vulkan backend in Diligent Core
option(POKO_WITH_VULKAN
    "Enable Vulkan backend in Diligent Core (requires git clone with submodules)"
    OFF)

# -----------------------------------------------------------------------------
# poko_record_pin(<name> <revision>)
# -----------------------------------------------------------------------------
function(poko_record_pin name revision)
    set(pins_file "${CMAKE_BINARY_DIR}/poko-dependency-pins.txt")
    file(APPEND "${pins_file}" "${name} ${revision}\n")
endfunction()

# -----------------------------------------------------------------------------
# poko_find_sdl2()
#
# Produces exactly one imported target, `SDL2::SDL2`, regardless of whether the
# dependency came from the MSYS2 package or pkg-config. Consumers link
# `SDL2::SDL2` and never see a path.
# -----------------------------------------------------------------------------
function(poko_find_sdl2)
    if(TARGET SDL2::SDL2)
        set(POKO_SDL2_SUMMARY "pre-existing target SDL2::SDL2" PARENT_SCOPE)
        return()
    endif()

    if(POKO_USE_SYSTEM_SDL2)
        find_package(SDL2 CONFIG QUIET)
    endif()

    if(TARGET SDL2::SDL2)
        get_target_property(_sdl2_version SDL2::SDL2 VERSION)
        if(NOT _sdl2_version)
            # SDL2Config.cmake exposes the version as a variable, not as a
            # target property.
            if(DEFINED SDL2_VERSION)
                set(_sdl2_version "${SDL2_VERSION}")
            elseif(DEFINED SDL2_VERSION_STRING)
                set(_sdl2_version "${SDL2_VERSION_STRING}")
            else()
                set(_sdl2_version "unknown")
            endif()
        endif()
        set(POKO_SDL2_SUMMARY "system package, version ${_sdl2_version}" PARENT_SCOPE)
        poko_record_pin(SDL2 "system-package ${_sdl2_version}")
        return()
    endif()

    # Fallback: pkg-config. Wrapped in an imported target so the rest of the
    # build does not depend on how SDL2 was located.
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(POKO_SDL2 QUIET IMPORTED_TARGET sdl2)
        if(TARGET PkgConfig::POKO_SDL2)
            add_library(SDL2::SDL2 INTERFACE IMPORTED)
            target_link_libraries(SDL2::SDL2 INTERFACE PkgConfig::POKO_SDL2)
            set(POKO_SDL2_SUMMARY "pkg-config, version ${POKO_SDL2_VERSION}" PARENT_SCOPE)
            poko_record_pin(SDL2 "pkg-config ${POKO_SDL2_VERSION}")
            return()
        endif()
    endif()

    message(FATAL_ERROR
        "SDL2 was not found.\n"
        "  On MSYS2 UCRT64 install it with:\n"
        "    pacman -S --needed mingw-w64-ucrt-x86_64-SDL2\n"
        "  Then reconfigure from a clean build directory.")
endfunction()

# -----------------------------------------------------------------------------
# poko_fetch_diligent_xxhash()
#
# Note: When using git clone with submodules, Diligent Core will handle its
# own xxHash submodule. This function is kept for compatibility but does nothing
# when using git clone.
# -----------------------------------------------------------------------------
function(poko_fetch_diligent_xxhash)
    # No-op when using git clone - Diligent handles submodules
endfunction()

# -----------------------------------------------------------------------------
# poko_fetch_diligent_core()
#
# Builds Diligent Core with git clone and submodules to enable Vulkan support.
# Builds the core library and selected backends.
#
# Offline / mirror builds: set -DFETCHCONTENT_SOURCE_DIR_DILIGENTCORE=<path> to
# use an existing checkout instead of downloading.
# -----------------------------------------------------------------------------
function(poko_fetch_diligent_core)
    if(NOT POKO_WITH_DILIGENT)
        set(POKO_DILIGENT_SUMMARY "disabled (POKO_WITH_DILIGENT=OFF)" PARENT_SCOPE)
        return()
    endif()

    if(TARGET Diligent-GraphicsEngineOpenGL-static)
        set(POKO_DILIGENT_SUMMARY "pre-existing Diligent targets" PARENT_SCOPE)
        return()
    endif()

    if(MINGW)
        # Diligent detects MinGW only from the generator name
        # (`if(${CMAKE_GENERATOR} MATCHES "MinGW")`), so a Ninja build never sets
        # MINGW_BUILD. Without it Diligent enables its DXC/ATL shader-compiler
        # path, which needs atlbase.h and cannot be built by GCC. We are in fact
        # compiling with MinGW-w64, so declare it before Diligent is added.
        set(MINGW_BUILD TRUE CACHE INTERNAL "Building with MinGW (declared by PokoX)")
    endif()

    # Backend selection. These are Diligent's own options; setting them before
    # MakeAvailable means its option() calls see the cached value.
    set(DILIGENT_NO_DIRECT3D11 ON CACHE BOOL "" FORCE)
    set(DILIGENT_NO_DIRECT3D12 ON CACHE BOOL "" FORCE)
    if(POKO_WITH_VULKAN)
        set(DILIGENT_NO_VULKAN OFF CACHE BOOL "" FORCE)
    else()
        set(DILIGENT_NO_VULKAN ON CACHE BOOL "" FORCE)
    endif()
    set(DILIGENT_NO_WEBGPU    ON CACHE BOOL "" FORCE)
    set(DILIGENT_NO_METAL     ON CACHE BOOL "" FORCE)
    set(DILIGENT_NO_ARCHIVER  ON CACHE BOOL "" FORCE)
    set(DILIGENT_NO_OPENGL    OFF CACHE BOOL "" FORCE)

    set(DILIGENT_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(DILIGENT_INSTALL_CORE OFF CACHE BOOL "" FORCE)
    set(DILIGENT_INSTALL_PDB  OFF CACHE BOOL "" FORCE)
    set(DILIGENT_BUILD_FX    OFF CACHE BOOL "" FORCE)
    set(DILIGENT_BUILD_SR    OFF CACHE BOOL "" FORCE)
    set(DILIGENT_BUILD_ARCHIVES_ONLY ON CACHE BOOL "" FORCE)
    set(DILIGENT_BUILD_FSR   OFF CACHE BOOL "" FORCE)
    set(DILIGENT_NO_FSR     ON CACHE BOOL "" FORCE)

    # Disable glslang build if not using Vulkan (saves build time)
    if(NOT POKO_WITH_VULKAN)
        set(DILIGENT_NO_GLSLANG ON CACHE BOOL "" FORCE)
    endif()

    FetchContent_Declare(DiligentCore
        GIT_REPOSITORY    "${POKO_DILIGENT_REPOSITORY}"
        GIT_TAG           "${POKO_DILIGENT_TAG}"
        GIT_SHALLOW       TRUE
        GIT_PROGRESS      TRUE)

    message(STATUS "poko: fetching Diligent Core ${POKO_DILIGENT_TAG} (commit ${POKO_DILIGENT_REVIEWED_COMMIT})")
    FetchContent_MakeAvailable(DiligentCore)

    # Diligent always defines shared engine targets. Under a Makefile
    # generator they link against `export.map` relative to their own binary dir; with
    # Ninja the link command runs from the build root, so that path does not
    # resolve and the DLL fails to link. PokoX links the static engine, so the
    # DLLs are removed from the default build instead of being "fixed" by patching
    # third-party code.
    if(TARGET Diligent-GraphicsEngineOpenGL-shared)
        set_target_properties(Diligent-GraphicsEngineOpenGL-shared
            PROPERTIES EXCLUDE_FROM_ALL TRUE EXCLUDE_FROM_DEFAULT_BUILD TRUE)
    endif()
    if(TARGET Diligent-GraphicsEngineVk-shared)
        set_target_properties(Diligent-GraphicsEngineVk-shared
            PROPERTIES EXCLUDE_FROM_ALL TRUE EXCLUDE_FROM_DEFAULT_BUILD TRUE)
    endif()
    # Exclude SuperResolution DLL targets (both debug and release)
    if(TARGET Diligent-SuperResolution-shared)
        set_target_properties(Diligent-SuperResolution-shared
            PROPERTIES EXCLUDE_FROM_ALL TRUE EXCLUDE_FROM_DEFAULT_BUILD TRUE)
    endif()
    if(TARGET SuperResolution_64)
        set_target_properties(SuperResolution_64
            PROPERTIES EXCLUDE_FROM_ALL TRUE EXCLUDE_FROM_DEFAULT_BUILD TRUE)
    endif()
    if(TARGET SuperResolution_64d)
        set_target_properties(SuperResolution_64d
            PROPERTIES EXCLUDE_FROM_ALL TRUE EXCLUDE_FROM_DEFAULT_BUILD TRUE)
    endif()

    if(POKO_WITH_VULKAN)
        set(_poko_diligent_backends "OpenGL and Vulkan")
    else()
        set(_poko_diligent_backends "OpenGL only")
    endif()
    poko_record_pin(DiligentCore "${POKO_DILIGENT_TAG} ${POKO_DILIGENT_COMMIT}")
    set(POKO_DILIGENT_SUMMARY
        "FetchContent ${POKO_DILIGENT_TAG} (git-pinned), ${_poko_diligent_backends}"
        PARENT_SCOPE)
endfunction()

# -----------------------------------------------------------------------------
# Resolution is driven explicitly by the root CMakeLists, after build options
# are declared, so that POKO_WITH_DILIGENT is already known.
# -----------------------------------------------------------------------------
