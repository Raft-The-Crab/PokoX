# =============================================================================
# M1 Demo Application
#
# Demonstrates the Poko Engine foundation.
# =============================================================================

cmake_minimum_required(VERSION 3.24)

add_executable(poko_m1_demo
    main.cpp
)

target_link_libraries(poko_m1_demo
    PRIVATE
        poko_core
)

# Copy runtime DLLs on Windows
if(WIN32)
    add_custom_command(TARGET poko_m1_demo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL2::SDL2>
            $<TARGET_FILE_DIR:poko_m1_demo>
        COMMENT "Bundling SDL2 runtime DLL for poko_m1_demo"
    )
endif()

# Add to CTest
add_test(NAME m1.demo
    COMMAND poko_m1_demo --frames 30
)
