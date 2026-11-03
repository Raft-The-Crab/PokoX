# =============================================================================
# M1 — Engine Foundation
#
# Milestone 1 implements the first reusable PokoX engine foundation.
# =============================================================================

# M1 demo application
add_subdirectory(demo)

# M1 compile-only test: public headers should not require Diligent
add_executable(test_public_headers
    test_public_headers.cpp
)

target_link_libraries(test_public_headers
    PRIVATE
        poko_core
        poko_platform
        poko_renderer
)

# This is a compile-only test; no need to actually run it
add_test(NAME m1.public_headers.compile
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target test_public_headers
)

# M1 renderer API behavior tests
add_executable(test_renderer_api
    test_renderer_api.cpp
)

target_link_libraries(test_renderer_api
    PRIVATE
        poko_renderer
)

add_test(NAME m1.renderer_api
    COMMAND test_renderer_api
)

# M1 lifecycle tests
add_executable(test_lifecycle
    test_lifecycle.cpp
)

target_link_libraries(test_lifecycle
    PRIVATE
        poko_core
)

add_test(NAME m1.lifecycle
    COMMAND test_lifecycle
)

# M1 resize behavior tests
add_executable(test_resize
    test_resize.cpp
)

target_link_libraries(test_resize
    PRIVATE
        poko_renderer
)

add_test(NAME m1.resize
    COMMAND test_resize
)
