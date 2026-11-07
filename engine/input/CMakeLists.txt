# =============================================================================
# Poko Input Module
#
# Provides input abstraction for keyboard, mouse, and gamepad devices.
# SDL2 implementation is used for cross-platform support.
# =============================================================================

cmake_minimum_required(VERSION 3.24)

project(poko_input
    VERSION 0.1.0
    DESCRIPTION "PokoX input abstraction layer"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# -----------------------------------------------------------------------------
# Public API headers
# -----------------------------------------------------------------------------
set(POKO_INPUT_PUBLIC_HEADERS
    public/poko/input/key_codes.h
    public/poko/input/keyboard.h
    public/poko/input/mouse.h
    public/poko/input/gamepad.h
    public/poko/input/input_manager.h
    public/poko/input/input_mapping.h
    public/poko/input/input_recorder.h
    public/poko/input/virtual_input.h
    public/poko/input/input.h
)

# -----------------------------------------------------------------------------
# Source files organized by component
# -----------------------------------------------------------------------------
set(POKO_INPUT_SOURCES
    src/keyboard/keyboard.cpp
    src/mouse/mouse.cpp
    src/gamepad/gamepad.cpp
    src/input_manager/input_manager.cpp
    src/input_mapping/input_mapping.cpp
    src/input_recorder/input_recorder.cpp
    src/virtual_input/virtual_input.cpp
)

# -----------------------------------------------------------------------------
# Create the input library
# -----------------------------------------------------------------------------
add_library(poko_input STATIC
    ${POKO_INPUT_PUBLIC_HEADERS}
    ${POKO_INPUT_SOURCES}
)

target_include_directories(poko_input
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/public>
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

# Link dependencies
target_link_libraries(poko_input
    PUBLIC
        poko_math
    PRIVATE
        SDL2::SDL2
)

# Set warning level per target
if(MSVC)
    target_compile_options(poko_input PRIVATE /W4)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(poko_input PRIVATE -Wall -Wextra -Wpedantic)
endif()

# -----------------------------------------------------------------------------
# Tests (controlled by root POKO_BUILD_INPUT option)
# -----------------------------------------------------------------------------
if(POKO_BUILD_INPUT)
    enable_testing()

    # Unit tests
    add_executable(test_input
        tests/test_input.cpp
    )
    target_link_libraries(test_input
        PRIVATE
            poko_input
    )
    add_test(NAME test_input COMMAND test_input)

    # Demo
    add_executable(demo_input
        demo/main.cpp
    )
    target_link_libraries(demo_input
        PRIVATE
            poko_input
            SDL2::SDL2
    )
endif()

# -----------------------------------------------------------------------------
# Install rules
# -----------------------------------------------------------------------------
install(TARGETS poko_input
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES ${POKO_INPUT_PUBLIC_HEADERS}
    DESTINATION include/poko/input
)
