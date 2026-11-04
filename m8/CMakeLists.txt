# =============================================================================
# M8 — Render Passes & Framebuffers
#
# Milestone 8 adds render-to-texture capabilities, multiple render targets (MRT),
# and framebuffer object management for advanced rendering techniques.
# =============================================================================

# M8 framebuffer tests
add_executable(test_framebuffer
    test_framebuffer.cpp
)

target_link_libraries(test_framebuffer
    PRIVATE
        poko_renderer
)

add_test(NAME m8.framebuffer
    COMMAND test_framebuffer
)

set_tests_properties(m8.framebuffer PROPERTIES
    TIMEOUT 10
    LABELS "m8;renderer"
)

# M8 integration tests
add_executable(test_integration
    test_integration.cpp
)

target_link_libraries(test_integration
    PRIVATE
        poko_renderer
)

add_test(NAME m8.integration
    COMMAND test_integration
)

set_tests_properties(m8.integration PROPERTIES
    TIMEOUT 15
    LABELS "m8;renderer;integration"
)

# M8 demo - render-to-texture
add_subdirectory(demo)
