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
#   DiligentCore  | FetchContent, pinned tag v2.5.6       | Apache-2.0
# =============================================================================
include_guard(GLOBAL)
include(FetchContent)

# -----------------------------------------------------------------------------
# Dependency pins
# -----------------------------------------------------------------------------

# Tag is used for a shallow clone; the SHA is the *reviewed* commit that the tag
# must resolve to. A mismatch is reported loudly instead of silently building
# different code than the one that was evaluated.
set(POKO_DILIGENT_GIT_REPOSITORY "https://github.com/DiligentGraphics/DiligentCore.git"
    CACHE STRING "Diligent Core repository URL")
set(POKO_DILIGENT_GIT_TAG "v2.5.6"
    CACHE STRING "Diligent Core tag or commit to build")
set(POKO_DILIGENT_PINNED_SHA "b036337d68be2353c9950a85929acf796b9a6d50"
    CACHE STRING "Commit SHA that POKO_DILIGENT_GIT_TAG is reviewed against")

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
        poko_record_pin(SDL2 "system-package")
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
# poko_fetch_diligent_core()
#
# Builds only the pieces M0 needs: the core library and the OpenGL backend.
# Direct3D, Vulkan, WebGPU, Metal and the archiver are disabled to keep the
# dependency build small and to keep unsupported backends out of the binary.
# -----------------------------------------------------------------------------
function(poko_fetch_diligent_core)
    if(NOT POKO_WITH_DILIGENT)
        set(POKO_DILIGENT_SUMMARY "disabled (POKO_WITH_DILIGENT=OFF)" PARENT_SCOPE)
        return()
    endif()

    if(TARGET Diligent-GraphicsEngineOpenGL)
        set(POKO_DILIGENT_SUMMARY "pre-existing Diligent targets" PARENT_SCOPE)
        return()
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
        GIT_REPOSITORY "${POKO_DILIGENT_GIT_REPOSITORY}"
        GIT_TAG        "${POKO_DILIGENT_GIT_TAG}"
        GIT_SHALLOW    TRUE
        GIT_PROGRESS   TRUE)

    FetchContent_MakeAvailable(DiligentCore)

    # Verify the revision actually built matches the reviewed commit.
    find_package(Git QUIET)
    set(_diligent_rev "unknown")
    if(Git_FOUND AND EXISTS "${diligentcore_SOURCE_DIR}/.git")
        execute_process(
            COMMAND "${GIT_EXECUTABLE}" rev-parse HEAD
            WORKING_DIRECTORY "${diligentcore_SOURCE_DIR}"
            OUTPUT_VARIABLE _diligent_rev
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET)
    endif()

    if(_diligent_rev STREQUAL "unknown")
        message(WARNING
            "poko: could not verify the Diligent Core revision (git metadata missing). "
            "Expected ${POKO_DILIGENT_PINNED_SHA}.")
    elseif(NOT _diligent_rev STREQUAL POKO_DILIGENT_PINNED_SHA)
        message(WARNING
            "poko: Diligent Core revision mismatch.\n"
            "  tag '${POKO_DILIGENT_GIT_TAG}' resolved to ${_diligent_rev}\n"
            "  reviewed pin is            ${POKO_DILIGENT_PINNED_SHA}\n"
            "  The tag may have moved upstream. Update the pin only after re-reviewing.")
    endif()

    poko_record_pin(DiligentCore "${_diligent_rev}")
    set(POKO_DILIGENT_SUMMARY
        "FetchContent ${POKO_DILIGENT_GIT_TAG} (${_diligent_rev}), OpenGL backend only"
        PARENT_SCOPE)
endfunction()

# -----------------------------------------------------------------------------
# Resolution is driven explicitly by the root CMakeLists, after build options
# are declared, so that POKO_WITH_DILIGENT is already known.
# -----------------------------------------------------------------------------
