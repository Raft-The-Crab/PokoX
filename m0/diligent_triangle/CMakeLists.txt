add_executable(poko_m0_diligent_triangle main.cpp)

# The static engine is linked on purpose: Diligent's shared OpenGL library has a
# broken MinGW link under Ninja (see cmake/modules/PokoDependencies.cmake), and a
# static link also keeps the M0 probe to a single executable with no DLL to ship.
target_link_libraries(poko_m0_diligent_triangle PRIVATE
    SDL2::SDL2
    Diligent-GraphicsEngineOpenGL-static)

poko_set_warnings(poko_m0_diligent_triangle)
poko_bundle_windows_runtime(poko_m0_diligent_triangle)

if(BUILD_TESTING)
    # Renders 30 frames, reads the back buffer back and checks that the triangle
    # actually covers the expected pixels. No human required.
    add_test(NAME m0.diligent_triangle.self_test
             COMMAND poko_m0_diligent_triangle --self-test --frames 30)
    set_tests_properties(m0.diligent_triangle.self_test PROPERTIES
        LABELS "m0;diligent;opengl"
        TIMEOUT 120
        PASS_REGULAR_EXPRESSION "PASS")

    add_test(NAME m0.diligent_triangle.run
             COMMAND poko_m0_diligent_triangle --frames 60)
    set_tests_properties(m0.diligent_triangle.run PROPERTIES
        LABELS "m0;diligent;opengl"
        TIMEOUT 120
        PASS_REGULAR_EXPRESSION "PASS")
endif()
