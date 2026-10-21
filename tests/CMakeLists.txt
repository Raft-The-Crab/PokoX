# Test executables will be added here in PKX-002 and subsequent tasks

# Enable testing in this subdirectory
enable_testing()

# PokoCore test executable
add_executable(PokoCoreTests
    unit/test_main.cpp
    unit/core/test_result.cpp
    unit/logger/test_logger.cpp
    unit/ids/test_entity_id.cpp
    unit/ids/test_resource_id.cpp
    unit/ids/test_entity_pool.cpp
)

# Link against PokoCore and build options
target_link_libraries(PokoCoreTests PRIVATE PokoCore pokox_build_options)

# Add include directory for test headers
target_include_directories(PokoCoreTests PRIVATE
    ${CMAKE_SOURCE_DIR}/engine/core/include
)

# Register test with CTest
add_test(NAME CoreTest COMMAND PokoCoreTests)

# Set test properties
set_tests_properties(CoreTest PROPERTIES
    LABELS "unit"
    TIMEOUT 30
)

# PokoPlatform test executable (Windows only)
if(WIN32)
    add_executable(PokoPlatformTests
        unit/platform/windows/test_windows_window.cpp
    )

    # Link against PokoPlatform and build options
    target_link_libraries(PokoPlatformTests PRIVATE PokoPlatform pokox_build_options)

    # Add include directories
    target_include_directories(PokoPlatformTests PRIVATE
        ${CMAKE_SOURCE_DIR}/engine/core/include
        ${CMAKE_SOURCE_DIR}/engine/platform/include
    )

    # Register test with CTest
    add_test(NAME PlatformTest COMMAND PokoPlatformTests)

    # Set test properties
    set_tests_properties(PlatformTest PROPERTIES
        LABELS "unit"
        TIMEOUT 30
    )
endif()

