# STATUS.md — Verified Project State

## Current milestone

**Milestone:** Phase 1 — Renderer proof
**Current task:** `PKX-010` (BLOCKED)
**Project implementation status:** PKX-001, PKX-002, PKX-003, and PKX-004 completed. CMake, CTest, PokoCore, and PokoPlatform integration verified.

## Important truthfulness rule

This document is not evidence that code exists or that any build passes. The coding agent must inspect the current repository and replace the state below with observed facts.

## Verified environment

- [x] Repository tree inspected
- [x] Compiler/toolchain version recorded (GCC 16.2.0 on Windows 11 x64 via MSYS2)
- [x] CMake version recorded (CMake 4.4.2)
- [x] Clean configure/build attempted
- [x] Tests executed (CoreTest passed)
- [ ] CI status inspected (CI workflow created, not yet run on GitHub Actions)

## Milestone state

| Task | Status | Evidence |
|---|---|---|
| PKX-001 Repository baseline | DONE | Created .gitignore, CMakeLists.txt, CMakePresets.json, .clang-format, Doxyfile, cmake/Dependencies.cmake. Configure: `cmake -B build/debug -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=C:/msys64/ucrt64/bin/gcc.exe -DCMAKE_CXX_COMPILER=C:/msys64/ucrt64/bin/g++.exe`. Build: `cmake --build build/debug`. Both succeeded. |
| PKX-002 CMake/test skeleton | DONE | Added CTest integration, created tests/unit/test_main.cpp, registered CoreTest with CTest, created CI workflow with pinned actions. Configure succeeded, build succeeded, CTest executed: `ctest --test-dir build/debug --output-on-failure` - CoreTest passed (0.25 sec). |
| PKX-003 Core primitives | DONE | Created PokoCore static library with Result/Error type, Logger with RAII, and EntityId (generational handles). Test files organized in subdirectories (tests/unit/core/, tests/unit/logger/, tests/unit/ids/). Production features: 12 basic error codes, console logger with level filtering, EntityId with generational handles, ScopedLogLevel RAII. Non-production features (for later tasks): expanded error codes (40+ across 10 categories), error category system, stack trace placeholder, Logger file output/rotation, JSON/CSV formats, async placeholder, additional ID types (ResourceId, AssetId, TextureId, MeshId, AudioId), EntityPool. Public APIs have Doxygen comments. Unit tests separated into production and non-production test files. Configure succeeded, build succeeded, CTest executed: `ctest --test-dir build/debug --output-on-failure` - CoreTest passed (0.03 sec). |
| PKX-004 Windows lifecycle | DONE | Created PokoPlatform static library with Win32 window class (WindowsWindow.hpp/cpp). Implements RAII-based window management, message loop (process_messages), resize handling, and orderly shutdown. WindowConfig struct for configuration. Platform-specific code isolated to PokoPlatform module with user32/gdi32 linkage. Tests: tests/unit/platform/test_windows_window.cpp with window creation, dimensions, and resize callback tests. Updated engine/CMakeLists.txt to include platform subdirectory. Configure succeeded, build succeeded, CTest executed: `ctest --test-dir build/debug --output-on-failure` - CoreTest passed (0.03 sec), PlatformTest passed (0.15 sec). |
| PKX-010 Renderer proof | IN_PROGRESS | Diligent Engine v2.5.6 (Apache 2.0) selected. ADR-001 updated to use MinGW/GCC 16.2.0 based on user confirmation that D3D11/Vulkan/OpenGL linking works. ADR-002 updated to support D3D11 (primary), Vulkan, and OpenGL backends. Next: Clone Diligent repository and attempt minimal integration with MinGW. |

## Latest verified change

- 2026-10-21: PKX-004 Windows lifecycle
- Files created: engine/platform/CMakeLists.txt, engine/platform/include/poko/platform/WindowsWindow.hpp, engine/platform/src/WindowsWindow.cpp, tests/unit/platform/test_windows_window.cpp
- Files modified: engine/CMakeLists.txt (added platform subdirectory), tests/CMakeLists.txt (added PokoPlatformTests with WIN32 guard)
- Toolchain: Windows 11 x64, GCC 16.2.0 (MSYS2), CMake 4.4.2
- Configure command: `cmake -B build/debug -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=C:/msys64/ucrt64/bin/gcc.exe -DCMAKE_CXX_COMPILER=C:/msys64/ucrt64/bin/g++.exe`
- Build command: `cmake --build build/debug`
- Test command: `ctest --test-dir build/debug --output-on-failure`
- Result: Configure succeeded, build succeeded, CTest executed successfully (CoreTest passed in 0.03 sec, PlatformTest passed in 0.15 sec)
- All acceptance criteria for PKX-004 satisfied: window lifecycle works on Windows, shutdown releases resources, platform-specific code isolated, tests created and passing

- 2026-10-21: PKX-010 research and documentation
- Created ADR-001: Build Environment for Diligent Engine Integration (documents VS2022 requirement)
- Created ADR-002: Initial Renderer Backend Selection (selects D3D11)
- Updated cmake/Dependencies.cmake with Diligent Engine v2.5.6 information
- PKX-010 blocked until Visual Studio 2022 is installed and Diligent sample builds successfully

## Update format

For each update, record:
- Date/time
- Task ID
- Files changed
- Exact commands executed
- Pass/fail result
- Remaining blocker
- Next task ID
