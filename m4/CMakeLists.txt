# =============================================================================
# M4 — Index Buffers and Indexed Mesh Rendering
#
# Milestone 4 adds index buffer API and indexed drawing.
# =============================================================================

# M4 index buffer API tests
add_executable(test_index_buffer
    test_index_buffer.cpp
)

target_link_libraries(test_index_buffer
    PRIVATE
        poko_renderer
)

add_test(NAME m4.index_buffer
    COMMAND test_index_buffer
)

set_tests_properties(m4.index_buffer PROPERTIES
    TIMEOUT 10
    LABELS "m4;renderer"
)

# M4 demo (updates M3 demo to use index buffers)
add_subdirectory(demo)
