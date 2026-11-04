# =============================================================================
# M9 — Advanced Drawing & Performance Benchmarks
#
# Milestone 9 adds instanced rendering, indirect drawing, and performance
# measurement utilities for efficient batched rendering and GPU-driven draw calls.
# =============================================================================

# M9 instanced drawing tests
add_executable(test_instanced_drawing
    test_instanced_drawing.cpp
)

target_link_libraries(test_instanced_drawing
    PRIVATE
        poko_renderer
)

add_test(NAME m9.instanced_drawing
    COMMAND test_instanced_drawing
)

set_tests_properties(m9.instanced_drawing PROPERTIES
    TIMEOUT 10
    LABELS "m9;renderer"
)

# M9 indirect drawing tests
add_executable(test_indirect_drawing
    test_indirect_drawing.cpp
)

target_link_libraries(test_indirect_drawing
    PRIVATE
        poko_renderer
)

add_test(NAME m9.indirect_drawing
    COMMAND test_indirect_drawing
)

set_tests_properties(m9.indirect_drawing PROPERTIES
    TIMEOUT 10
    LABELS "m9;renderer"
)

# M9 performance benchmark tests
add_executable(test_performance
    test_performance.cpp
)

target_link_libraries(test_performance
    PRIVATE
        poko_renderer
)

add_test(NAME m9.performance
    COMMAND test_performance
)

set_tests_properties(m9.performance PROPERTIES
    TIMEOUT 30
    LABELS "m9;renderer;performance"
)

# M9 demo - instanced and indirect drawing
add_subdirectory(demo)
