# =============================================================================
# M10 Demo — Skinned GLB Character
# =============================================================================

add_executable(poko_m10_demo
    main.cpp
)

target_link_libraries(poko_m10_demo
    PRIVATE
        poko_core
        poko_world
        poko_renderer
        poko_math
        SDL2::SDL2
)

target_include_directories(poko_m10_demo
    PRIVATE
        ${CMAKE_SOURCE_DIR}/engine/platform/public
)

# Copy SDL2 DLL on Windows
if(WIN32)
    add_custom_command(TARGET poko_m10_demo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL2::SDL2>
            $<TARGET_FILE_DIR:poko_m10_demo>
        COMMENT "Bundling SDL2 runtime DLL for poko_m10_demo"
    )
endif()

# Automated test (headless frame-limited run). The demo resolves asset paths
# relative to the repo root, so run the test from the source directory.
add_test(NAME m10.demo
    COMMAND poko_m10_demo --frames 30
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

set_tests_properties(m10.demo PROPERTIES
    TIMEOUT 60
    LABELS "m10;graphics"
)
