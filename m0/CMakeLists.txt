# =============================================================================
# M0 — technical feasibility applications.
#
# These targets exist to answer environment questions, not to become engine
# code. They are intentionally small, dependency-light and disposable; the real
# modules start in M1 under engine/.
# =============================================================================

# Proves the CMake/Ninja/compiler pipeline works with no third-party code.
add_subdirectory(smoke)

# M0-B: SDL2 window, input events, clean shutdown.
add_subdirectory(sdl2_window)

# M0-D: C ABI boundary and an external C caller.
add_subdirectory(abi)

# M0-C: Diligent Core OpenGL device on an SDL2-owned window.
if(POKO_WITH_DILIGENT)
    add_subdirectory(diligent_triangle)
else()
    message(STATUS "poko: M0 Diligent triangle disabled (POKO_WITH_DILIGENT=OFF)")
endif()
