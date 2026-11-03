# M0-D: C ABI library plus an external C caller.
#
# The library is C++20; the caller is C11 compiled as C. That split is the point
# of the test — a leaked C++ construct in the header breaks the caller build.

add_library(poko_abi STATIC src/engine_abi.cpp)

target_include_directories(poko_abi
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

target_compile_features(poko_abi PUBLIC cxx_std_20)
poko_set_warnings(poko_abi)

add_executable(poko_m0_abi_caller caller/main.c)
target_link_libraries(poko_m0_abi_caller PRIVATE poko_abi)
poko_set_warnings(poko_m0_abi_caller)

# Fail the build (not the test run) if the header stops being valid C.
set_target_properties(poko_m0_abi_caller PROPERTIES C_STANDARD 11 C_STANDARD_REQUIRED ON)

if(BUILD_TESTING)
    add_test(NAME m0.abi.c_caller COMMAND poko_m0_abi_caller)
    set_tests_properties(m0.abi.c_caller PROPERTIES
        LABELS "m0;abi"
        TIMEOUT 30
        PASS_REGULAR_EXPRESSION "PASS")
endif()
