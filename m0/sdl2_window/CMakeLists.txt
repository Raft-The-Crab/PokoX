add_executable(poko_m0_sdl2_window main.cpp)

# SDL_MAIN_HANDLED is defined in the source itself so the translation unit stays
# self-describing; defining it here as well would only produce a redefinition
# warning.
target_link_libraries(poko_m0_sdl2_window PRIVATE SDL2::SDL2)

poko_set_warnings(poko_m0_sdl2_window)
poko_bundle_windows_runtime(poko_m0_sdl2_window)

if(BUILD_TESTING)
    # Automated run: window + event loop, no human required. The self-test mode
    # injects synthetic key/mouse/resize events and asserts they were dispatched.
    add_test(NAME m0.sdl2_window.self_test
             COMMAND poko_m0_sdl2_window --self-test --frames 30)
    set_tests_properties(m0.sdl2_window.self_test PROPERTIES
        LABELS "m0;sdl2"
        TIMEOUT 60
        PASS_REGULAR_EXPRESSION "PASS")

    add_test(NAME m0.sdl2_window.run
             COMMAND poko_m0_sdl2_window --frames 60)
    set_tests_properties(m0.sdl2_window.run PROPERTIES
        LABELS "m0;sdl2"
        TIMEOUT 60
        PASS_REGULAR_EXPRESSION "PASS")
endif()
