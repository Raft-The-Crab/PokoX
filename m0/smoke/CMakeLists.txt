add_executable(poko_m0_smoke main.cpp)
poko_set_warnings(poko_m0_smoke)

set_target_properties(poko_m0_smoke PROPERTIES
    OUTPUT_NAME "poko_m0_smoke")

if(BUILD_TESTING)
    add_test(NAME m0.smoke COMMAND poko_m0_smoke)
    set_tests_properties(m0.smoke PROPERTIES
        LABELS "m0;toolchain"
        TIMEOUT 30)
endif()
