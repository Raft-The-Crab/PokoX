# =============================================================================
# M9 Demo — Advanced Drawing
#
# Demonstrates instanced rendering and indirect drawing for efficient
# batched geometry and GPU-driven draw calls.
# =============================================================================

add_executable(poko_m9_demo
    main.cpp
)

target_link_libraries(poko_m9_demo
    PRIVATE
        poko_core
        poko_renderer
        SDL2::SDL2
)

target_include_directories(poko_m9_demo
    PRIVATE
        ${CMAKE_SOURCE_DIR}/engine/platform/public
)

# Copy SDL2 DLL on Windows
if(WIN32)
    add_custom_command(TARGET poko_m9_demo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL2::SDL2>
            $<TARGET_FILE_DIR:poko_m9_demo>
        COMMENT "Bundling SDL2 runtime DLL for poko_m9_demo"
    )
endif()

# Automated test
add_test(NAME m9.demo
    COMMAND poko_m9_demo --frames 10
)

set_tests_properties(m9.demo PROPERTIES
    TIMEOUT 30
    LABELS "m9;graphics"
)
