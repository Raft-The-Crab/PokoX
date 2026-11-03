# =============================================================================
# PokoHelpers.cmake — small, target-scoped build helpers.
#
# Deliberately minimal: helpers exist so that per-target requirements
# (warnings, C++ standard, DLL bundling) are expressed once instead of being
# spread across directory-level global flags.
# =============================================================================
include_guard(GLOBAL)

# -----------------------------------------------------------------------------
# poko_add_component_if_present(<relative-path>)
#
# Adds a subdirectory only when it contains a CMakeLists.txt. Keeps the build
# honest: an unimplemented module is a one-line status message, never a hard
# configure error, and never a silently empty target.
# -----------------------------------------------------------------------------
function(poko_add_component_if_present relative_path)
    if(EXISTS "${PROJECT_SOURCE_DIR}/${relative_path}/CMakeLists.txt")
        add_subdirectory(
            "${PROJECT_SOURCE_DIR}/${relative_path}"
            "${CMAKE_BINARY_DIR}/${relative_path}")
    else()
        message(STATUS "poko: not implemented yet, skipped -> ${relative_path}")
    endif()
endfunction()

# -----------------------------------------------------------------------------
# poko_set_warnings(<target> [WERROR])
#
# Warnings are per-target and never global, so a third-party or generated
# target can opt out without weakening the rest of the build.
# -----------------------------------------------------------------------------
function(poko_set_warnings target)
    cmake_parse_arguments(arg "WERROR" "" "" ${ARGN})

    if(MSVC)
        set(poko_warnings /W4 /permissive-)
        if(arg_WERROR)
            list(APPEND poko_warnings /WX)
        endif()
    else()
        set(poko_warnings -Wall -Wextra -Wpedantic)
        if(arg_WERROR)
            list(APPEND poko_warnings -Werror)
        endif()
    endif()

    target_compile_options(${target} PRIVATE ${poko_warnings})
endfunction()

# -----------------------------------------------------------------------------
# poko_bundle_windows_runtime(<target>)
#
# Copies the DLLs an executable needs next to the executable, so it can be
# started outside the MSYS2 shell. Uses CMake's runtime-DLL generation where the
# imported targets provide DLL locations, with an explicit SDL2 fallback.
# -----------------------------------------------------------------------------
function(poko_bundle_windows_runtime target)
    if(NOT WIN32)
        return()
    endif()

    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy -t "$<TARGET_FILE_DIR:${target}>"
                "$<TARGET_RUNTIME_DLLS:${target}>"
        COMMAND_EXPAND_LISTS
        VERBATIM
        COMMENT "Bundling runtime DLLs for ${target}")
endfunction()

# -----------------------------------------------------------------------------
# poko_print_build_summary()
# -----------------------------------------------------------------------------
function(poko_print_build_summary)
    message(STATUS "")
    message(STATUS "PokoX ${PROJECT_VERSION} build summary")
    message(STATUS "  generator        : ${CMAKE_GENERATOR}")
    message(STATUS "  build type       : ${CMAKE_BUILD_TYPE}")
    message(STATUS "  C compiler       : ${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}")
    message(STATUS "  C++ compiler     : ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
    message(STATUS "  C++ standard     : C++${CMAKE_CXX_STANDARD}")
    message(STATUS "  SDL2             : ${POKO_SDL2_SUMMARY}")
    message(STATUS "  Diligent Core    : ${POKO_DILIGENT_SUMMARY}")
    message(STATUS "  M0 apps          : ${POKO_BUILD_M0}")
    message(STATUS "  tests            : ${BUILD_TESTING}")
    message(STATUS "  output directory : ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
    message(STATUS "")
endfunction()
