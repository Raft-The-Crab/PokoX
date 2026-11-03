# =============================================================================
# M7 — Constant Buffers
#
# Milestone 7 adds constant buffer (uniform buffer) API for shader uniform data.
# Supports dynamic updates for frame-varying data like matrices.
# =============================================================================

# M7 constant buffer tests
add_executable(test_constant_buffer
    test_constant_buffer.cpp
)

target_link_libraries(test_constant_buffer
    PRIVATE
        poko_renderer
)

add_test(NAME m7.constant_buffer
    COMMAND test_constant_buffer
)

set_tests_properties(m7.constant_buffer PROPERTIES
    TIMEOUT 10
    LABELS "m7;renderer"
)

# M7 demo - constant buffer with matrix transformation
add_subdirectory(demo)
