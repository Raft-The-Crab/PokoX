# =============================================================================
# M3 — Vertex Buffers and Minimal Mesh Rendering
#
# Milestone 3 adds vertex buffer API and uses vertex data instead of gl_VertexID.
# =============================================================================

# M3 vertex buffer API tests
add_executable(test_vertex_buffer
    test_vertex_buffer.cpp
)

target_link_libraries(test_vertex_buffer
    PRIVATE
        poko_renderer
)

add_test(NAME m3.vertex_buffer
    COMMAND test_vertex_buffer
)

set_tests_properties(m3.vertex_buffer PROPERTIES
    TIMEOUT 10
    LABELS "m3;renderer"
)

# M3 demo (updates M2 demo to use vertex buffers)
add_subdirectory(demo)
