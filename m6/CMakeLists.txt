# =============================================================================
# M6 — Materials and Shader Resource Bindings
#
# Milestone 6 adds material API for grouping texture-slot bindings.
# Full SRB creation requires pipeline resource signature (deferred).
# =============================================================================

# M6 material API tests
add_executable(test_material
    test_material.cpp
)

target_link_libraries(test_material
    PRIVATE
        poko_renderer
)

add_test(NAME m6.material
    COMMAND test_material
)

set_tests_properties(m6.material PROPERTIES
    TIMEOUT 10
    LABELS "m6;renderer"
)

# M6 demo - material creation only (full SRB binding deferred)
add_subdirectory(demo)
