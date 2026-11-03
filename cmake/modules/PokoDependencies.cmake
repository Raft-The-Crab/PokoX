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
set(POKO_DILIGENT_VERSION "v2.5.6")
set(POKO_DILIGENT_URL
    "https://codeload.github.com/DiligentGraphics/DiligentCore/tar.gz/refs/tags/${POKO_DILIGENT_VERSION}"
    CACHE STRING "Diligent Core release archive URL")
set(POKO_DILIGENT_URL_HASH
    "SHA256=abc190c05ee7e5ef2bba52fcbc5fdfe2256cce3435efba9cfe263a386653f671"
    CACHE STRING "Expected hash of POKO_DILIGENT_URL; empty disables verification")
# The commit that tag v2.5.6 resolved to when this pin was reviewed. GitHub
# archives for a tag are regenerated upstream, not stored, so the hash is the
# authoritative pin and this SHA is the human-readable traceability record.
set(POKO_DILIGENT_REVIEWED_COMMIT "b036337d68be2353c9950a85929acf796b9a6d50")

# --- xxHash (a Diligent Core third-party dependency) --------------------------
# Diligent's ThirdParty/xxHash is a git submodule, so it is empty in the release
# archive, while ThirdParty/CMakeLists.txt adds xxHash/cmake_unofficial
# unconditionally. Supplying the exact upstream (Diligent fork) commit and
# pre-creating `xxHash::xxhash` makes Diligent skip its own submodule block.
# The hash below was computed from a fresh download of this archive, not copied
# from documentation.
set(POKO_XXHASH_COMMIT "a57f6cce2698049863af8c25787084ae0489d849")
set(POKO_XXHASH_URL
    "https://codeload.github.com/DiligentGraphics/xxHash/tar.gz/${POKO_XXHASH_COMMIT}"
    CACHE STRING "xxHash archive URL required by Diligent Core")
set(POKO_XXHASH_URL_HASH
    "SHA256=50a63437bbc5dc34c62d00dfce92324d8f17e5d116a659e074a3e0a1d02b6f16"
    CACHE STRING "Expected hash of POKO_XXHASH_URL; empty disables verification")

# Windows desktop consumes the MSYS2 package so that exactly one SDL2 (and one
# SDL2 runtime DLL) exists in the process. Android will build SDL2 from a pinned
# source revision under its own toolchain, never mixing the two.
option(POKO_USE_SYSTEM_SDL2
    "Use the SDL2 package supplied by the toolchain (MSYS2 UCRT64) instead of building SDL2 from source"
    ON)

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
# Must run before Diligent is added. xxHash is a C-only project, so the knobs
# below are set as normal variables (not cache entries) with the same values
# Diligent itself would use: static library, no xxhsum executable, no install
# rules leaking into the PokoX install tree.
# -----------------------------------------------------------------------------
function(poko_fetch_diligent_xxhash)
    if(TARGET xxHash::xxhash)
        return()
    endif()

    set(BUILD_SHARED_LIBS      OFF)
    set(XXHASH_BUILD_XXHSUM    OFF)
    set(XXHASH_BUNDLED_MODE    ON)

    FetchContent_Declare(xxHash
        URL                   "${POKO_XXHASH_URL}"
        URL_HASH              "${POKO_XXHASH_URL_HASH}"
        SOURCE_SUBDIR         cmake_unofficial
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE)

    message(STATUS "poko: fetching xxHash ${POKO_XXHASH_COMMIT} (required by Diligent Core)")
    FetchContent_MakeAvailable(xxHash)

    if(NOT TARGET xxHash::xxhash)
        message(FATAL_ERROR
            "xxHash was fetched, but no xxHash::xxhash target was created; "
            "Diligent Core cannot be built without it.")
    endif()

    # A shared xxhash would need a runtime DLL next to every executable; the
    # static build is what Diligent expects, so refuse to continue otherwise.
    get_target_property(_poko_xxhash_type xxHash::xxhash TYPE)
    if(NOT _poko_xxhash_type STREQUAL "STATIC_LIBRARY")
        message(FATAL_ERROR
            "xxHash was built as '${_poko_xxhash_type}', expected STATIC_LIBRARY.")
    endif()

    poko_record_pin(xxHash "${POKO_XXHASH_COMMIT} ${POKO_XXHASH_URL_HASH}")
endfunction()

# -----------------------------------------------------------------------------
# poko_fetch_diligent_core()
#
# Builds only the pieces M0 needs: the core library and the OpenGL backend.
# Direct3D, Vulkan, WebGPU, Metal and the archiver are disabled to keep the
# dependency build small and to keep unsupported backends out of the binary.
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
    set(DILIGENT_NO_VULKAN    ON CACHE BOOL "" FORCE)
    set(DILIGENT_NO_WEBGPU    ON CACHE BOOL "" FORCE)
    set(DILIGENT_NO_METAL     ON CACHE BOOL "" FORCE)
    set(DILIGENT_NO_ARCHIVER  ON CACHE BOOL "" FORCE)
    set(DILIGENT_NO_OPENGL    OFF CACHE BOOL "" FORCE)

    set(DILIGENT_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(DILIGENT_INSTALL_CORE OFF CACHE BOOL "" FORCE)
    set(DILIGENT_INSTALL_PDB  OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(DiligentCore
        URL                   "${POKO_DILIGENT_URL}"
        URL_HASH              "${POKO_DILIGENT_URL_HASH}"
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE)

    message(STATUS "poko: fetching Diligent Core ${POKO_DILIGENT_VERSION} (pin ${POKO_DILIGENT_URL_HASH}; upstream commit ${POKO_DILIGENT_REVIEWED_COMMIT})")
    # Saturates the `ThirdParty/xxHash` submodule that the release archive
    # leaves empty. Must precede MakeAvailable(DiligentCore).
    poko_fetch_diligent_xxhash()
    FetchContent_MakeAvailable(DiligentCore)

    # Diligent always defines a shared OpenGL engine target. Under a Makefile
    # generator it links against `export.map` relative to its own binary dir; with
    # Ninja the link command runs from the build root, so that path does not
    # resolve and the DLL fails to link. PokoX links the static engine, so the
    # DLL is removed from the default build instead of being "fixed" by patching
    # third-party code.
    if(TARGET Diligent-GraphicsEngineOpenGL-shared)
        set_target_properties(Diligent-GraphicsEngineOpenGL-shared
            PROPERTIES EXCLUDE_FROM_ALL TRUE)
    endif()

    poko_record_pin(DiligentCore "${POKO_DILIGENT_VERSION} ${POKO_DILIGENT_URL_HASH}")
    set(POKO_DILIGENT_SUMMARY
        "FetchContent ${POKO_DILIGENT_VERSION} (content-pinned), OpenGL backend only"
        PARENT_SCOPE)
endfunction()

# -----------------------------------------------------------------------------
# Resolution is driven explicitly by the root CMakeLists, after build options
# are declared, so that POKO_WITH_DILIGENT is already known.
# -----------------------------------------------------------------------------
