// =============================================================================
// PokoX M0 — toolchain smoke test.
//
// Purpose: prove that the selected generator, compiler and C++20 settings
// produce a working executable, and record the exact toolchain that was used.
// It links nothing and touches no platform API, so a failure here is
// unambiguously a toolchain problem.
// =============================================================================

#include <array>
#include <bit>
#include <concepts>
#include <cstdint>
#include <cstdio>
#include <span>
#include <string_view>
#include <type_traits>

namespace {

struct Toolchain {
    const char* compiler;
    std::string_view version;
    long cpp_standard;
};

template <std::integral T>
constexpr T clamp_positive(T value) {
    return value < T{0} ? T{0} : value;
}

// C++20 concept + span + constexpr evaluation, exercised so that the standard
// level is observable rather than merely requested in CMake.
constexpr int sum(std::span<const int> values) {
    int total = 0;
    for (int value : values) {
        total += value;
    }
    return total;
}

}  // namespace

int main() {
    constexpr std::array<int, 5> numbers{1, 2, 3, 4, 5};
    static_assert(sum(numbers) == 15, "constexpr evaluation failed");
    static_assert(clamp_positive(-3) == 0, "concept-constrained template failed");
    static_assert(clamp_positive(7) == 7, "concept-constrained template failed");
    static_assert(std::endian::native == std::endian::little,
                  "PokoX currently assumes a little-endian host");

#if defined(__clang__)
    const Toolchain toolchain{"Clang",
                              std::string_view{__clang_version__},
                              __cplusplus};
#elif defined(__GNUC__)
    const Toolchain toolchain{"GCC",
                              std::string_view{__VERSION__},
                              __cplusplus};
#elif defined(_MSC_VER)
    const Toolchain toolchain{"MSVC", "see _MSC_VER", __cplusplus};
#else
    const Toolchain toolchain{"unknown", "unknown", __cplusplus};
#endif

    std::printf("PokoX M0 toolchain smoke test\n");
    std::printf("  compiler        : %s %.*s\n", toolchain.compiler,
                static_cast<int>(toolchain.version.size()), toolchain.version.data());
    std::printf("  __cplusplus     : %ldL\n", toolchain.cpp_standard);
    std::printf("  pointer size    : %zu bytes\n", sizeof(void*));
    std::printf("  long double     : %zu bytes\n", sizeof(long double));
    std::printf("  endianness      : %s\n",
                std::endian::native == std::endian::little ? "little" : "big");

    if (toolchain.cpp_standard < 202002L) {
        std::printf("FAIL: C++20 was not enabled (__cplusplus=%ldL)\n",
                    toolchain.cpp_standard);
        return 1;
    }

    std::printf("PASS\n");
    return 0;
}
