# =============================================================================
# M2 — Minimal Scene Rendering Vertical Slice
#
# Milestone 2 renders a simple triangle through the Poko renderer API.
# =============================================================================

# M2 pipeline API tests
add_executable(test_pipeline
    test_pipeline.cpp
)

target_link_libraries(test_pipeline
    PRIVATE
        poko_renderer
)

add_test(NAME m2.pipeline
    COMMAND test_pipeline
)

set_tests_properties(m2.pipeline PROPERTIES
    TIMEOUT 10
    LABELS "m2;renderer"
)

# M2 demo application
add_subdirectory(demo)
