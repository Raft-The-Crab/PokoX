add_executable(poko_m4_demo
    main.cpp
)

target_link_libraries(poko_m4_demo
    PRIVATE
        poko_core
        poko_renderer
)

target_include_directories(poko_m4_demo
    PRIVATE
        ${CMAKE_SOURCE_DIR}/engine/platform/public
)

target_link_libraries(poko_m4_demo
    PRIVATE
        SDL2::SDL2
)

# Bundle SDL2 runtime DLL for Windows
if(WIN32)
    add_custom_command(TARGET poko_m4_demo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:SDL2::SDL2>"
            "$<TARGET_FILE_DIR:poko_m4_demo>"
        COMMENT "Bundling SDL2 runtime DLL for poko_m4_demo"
    )
endif()

# Automated test
add_test(NAME m4.demo
    COMMAND poko_m4_demo --frames 10
)

set_tests_properties(m4.demo PROPERTIES
    TIMEOUT 30
    LABELS "m4;graphics"
)
