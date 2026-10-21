# Test executables will be added here in PKX-002 and subsequent tasks

# Enable testing in this subdirectory
enable_testing()

# Minimal test executable for CTest verification
add_executable(PokoCoreTests
    unit/test_main.cpp
)

# Link against build options
target_link_libraries(PokoCoreTests PRIVATE pokox_build_options)

# Register test with CTest
add_test(NAME CoreTest COMMAND PokoCoreTests)

# Set test properties
set_tests_properties(CoreTest PROPERTIES
    LABELS "unit"
    TIMEOUT 30
)

