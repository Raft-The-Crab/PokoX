# =============================================================================
# PokoX Math Library
#
# Provides vector, matrix, quaternion, and transform math operations for
# the PokoX engine. Optimized for game development use cases.
# =============================================================================

# Math library (header-only)
add_library(poko_math
    INTERFACE
)

target_include_directories(poko_math
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/engine/math/public>
        $<INSTALL_INTERFACE:include>
)

# Math library tests
add_executable(test_math
    ${CMAKE_CURRENT_SOURCE_DIR}/test_math.cpp
)

target_link_libraries(test_math
    PRIVATE
        poko_math
)

add_test(NAME math.tests
    COMMAND test_math
)

# Math library demo
add_executable(demo_math
    ${CMAKE_CURRENT_SOURCE_DIR}/demo_math.cpp
)

target_link_libraries(demo_math
    PRIVATE
        poko_math
)
