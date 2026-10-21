cmake_minimum_required(VERSION 3.24)

# Auto-detect generator based on available compilers
# If Visual Studio is available, use it (as specified in docs)
# Otherwise, fall back to available compiler (GCC/Clang)
if(WIN32)
    # Try to find Visual Studio
    find_program(CMAKE_C_COMPILER NAMES cl)
    find_program(CMAKE_CXX_COMPILER NAMES cl)
    
    if(NOT CMAKE_C_COMPILER OR NOT CMAKE_CXX_COMPILER)
        message(STATUS "Visual Studio not found, will use available compiler")
    endif()
endif()

project(PokoX
    VERSION 0.1.0
    DESCRIPTION "PokoX Game Engine and Platform"
    LANGUAGES CXX
)

# Set C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build options interface target
add_library(pokox_build_options INTERFACE)

# Common compiler warnings
target_compile_features(pokox_build_options INTERFACE cxx_std_20)

if(MSVC)
    target_compile_options(pokox_build_options INTERFACE
        /W4                  # High warning level
        /permissive-         # Disable non-conforming code
        /Zc:__cplusplus      # Correct __cplusplus macro
        /EHsc                # Exception handling
        /utf-8               # Use UTF-8 source
    )
else()
    target_compile_options(pokox_build_options INTERFACE
        -Wall
        -Wextra
        -Wpedantic
        -Werror=return-type
    )
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR})

# Add subdirectories
add_subdirectory(cmake)
add_subdirectory(engine)
add_subdirectory(apps)
add_subdirectory(tests)

# Enable testing
enable_testing()

# Add test discovery
include(CTest)

# Print configuration summary
message(STATUS "PokoX Version: ${PROJECT_VERSION}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
