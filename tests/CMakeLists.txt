# Test executables will be added here in PKX-002 and subsequent tasks

# Enable testing in this subdirectory
enable_testing()

# PokoCore test executable
add_executable(PokoCoreTests
    unit/test_main.cpp
    unit/test_result.cpp
    unit/test_logger.cpp
    unit/test_entity_id.cpp
    unit/test_resource_id.cpp
    unit/test_entity_pool.cpp
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

