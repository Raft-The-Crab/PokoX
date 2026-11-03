# 26 — Build Environment, MSYS2, CMake, and Dependency Management

**Status:** Proposed developer setup; exact dependency versions must be pinned after feasibility testing.  
**Windows shell:** MSYS2 UCRT64  
**Build system:** CMake + Ninja  
**Android:** Android Studio/NDK + CMake toolchain.

## 1. Windows toolchain decision

Use MSYS2 UCRT64 as the first Windows GCC environment. MSYS2 documentation recommends UCRT64 when unsure which environment to choose. Do not mix `MINGW64`, `UCRT64`, `CLANG64`, MSVC and MSYS runtime libraries casually in one binary. Decide the toolchain per build profile and keep dependencies built for the same compatible runtime/ABI.

## 2. Initial packages

In the UCRT64 terminal, update MSYS2 first and then install the toolchain. Package names can change; check the MSYS2 package index before setup.

```bash
pacman -Syu
# If the update asks to close/reopen the terminal, do so and repeat as instructed.

pacman -S --needed \
  mingw-w64-ucrt-x86_64-toolchain \
  mingw-w64-ucrt-x86_64-cmake \
  mingw-w64-ucrt-x86_64-ninja \
  mingw-w64-ucrt-x86_64-pkgconf \
  mingw-w64-ucrt-x86_64-SDL2 \
  mingw-w64-ucrt-x86_64-git
```

Add formatting, static analysis, debugging, sanitizer and Doxygen packages after checking compatible versions and availability. Avoid installing every optional tool before the first renderer prototype.

## 3. Repository layout

```text
PokoX/
  CMakeLists.txt
  cmake/
    toolchains/
    modules/
  third_party/
    DiligentCore/       # pinned submodule or FetchContent revision
    SDL2/               # pinned package/source strategy
  engine/
  applications/
  mute/
  server/
  tests/
  documentation/
```

Choose one dependency acquisition approach per dependency: pinned git submodule, vendored source with license notices, or package-manager dependency. Do not simultaneously link a system SDL2 and a separately compiled SDL2 without clear target isolation.

## 4. CMake target conventions

- Every module has a target and public/private include boundary.
- Use `target_link_libraries`, `target_compile_features`, and target-specific compile definitions.
- Keep global include directories and global compiler flags to a minimum.
- Use generator expressions or per-configuration properties for Debug/Release.
- Provide `BUILD_TESTING` and optional tools flags.
- Have a headless target that does not pull in SDL video or Diligent renderer.
- Export/install targets only when an SDK packaging requirement exists.
- Do not compile untrusted creator code as native C++ extensions.

Suggested targets are listed in `03_ENGINE_ARCHITECTURE.md`.

## 5. First Windows proof-of-build

The first milestone should not be a full game. It should:

1. Configure CMake with Ninja in UCRT64.
2. Build a tiny SDL2 window/input sample.
3. Build Diligent Core with the chosen compiler and OpenGL backend.
4. Create one GL context through a clearly owned window/context path.
5. Attach/initialize Diligent according to its documented interop path.
6. Render a triangle and textured quad.
7. Resize, minimize, restore and close.
8. Run under Debug and Release.
9. Confirm no ABI/runtime mismatch between dependencies.

Example project build:

```bash
cmake -S . -B build/windows-debug -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DPOKO_BUILD_CLIENT=ON \
  -DPOKO_BUILD_STUDIO=OFF \
  -DPOKO_BUILD_MOD=OFF \
  -DPOKO_BUILD_AUTHORITY=OFF

cmake --build build/windows-debug
ctest --test-dir build/windows-debug --output-on-failure
```

These options are proposed project options; add them to CMake only when implemented.

## 6. Android build strategy

Use Android Studio, Android SDK/NDK and CMake cross-compilation for supported ABIs. Keep the Android toolchain separate from MSYS2; MSYS2 is not the Android compiler. Build SDL2 for Android and verify the JNI/activity integration. Verify Diligent OpenGL ES device creation with a native Android surface. Test pause/resume, low-memory event, surface recreation, app termination and safe-area behavior.

Do not assume that the Windows CMake generator/toolchain file can be reused unchanged for Android. Use a clean Android build directory and NDK toolchain file.

## 7. Diligent/SDL context ownership

One subsystem must own context creation and thread affinity. Prototype whether SDL creates the window/context and Diligent attaches to it, or whether Diligent creates/owns the relevant context using the SDL native window. Do not let both systems create competing contexts. Document swap interval, resize, context loss and surface recreation behavior.

## 8. Dependency governance

For every third-party dependency record: name, upstream URL, pinned version/commit, license, platforms, binary/source mode, patches, known vulnerabilities, and upgrade test. Generate a software bill of materials for release builds where tooling permits. Keep license texts/notices in distributed applications. Never fetch an unpinned dependency at build time for release builds.

## 9. Testing and CI

CI should run format checks, configure/build, unit tests, package parser tests, MUTE tests and a minimal headless authority build. GPU/window integration tests may require dedicated self-hosted hardware; do not pretend a headless CI runner has validated GPU behavior. Store logs and artifacts without secrets.

## 10. References

- [MSYS2 environments](https://www.msys2.org/docs/environments/)
- [DiligentCore](https://github.com/DiligentGraphics/DiligentCore)
- [Diligent getting started](https://diligentgraphics.com/diligent-engine/getting-started/)
- [SDL2 overview](https://wiki.libsdl.org/SDL2/Introduction)
- [SDL2 Android](https://wiki.libsdl.org/SDL2/Android)
