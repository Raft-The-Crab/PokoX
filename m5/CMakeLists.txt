# =============================================================================
# M5 — Textures and Texture Sampling
#
# Milestone 5 adds texture creation API. Texture binding is deferred to M6.
# =============================================================================

# M5 texture API tests
add_executable(test_texture
    test_texture.cpp
)

target_link_libraries(test_texture
    PRIVATE
        poko_renderer
)

add_test(NAME m5.texture
    COMMAND test_texture
)

set_tests_properties(m5.texture PROPERTIES
    TIMEOUT 10
    LABELS "m5;renderer"
)

# M5 demo - texture creation only (binding deferred to M6)
add_subdirectory(demo)
