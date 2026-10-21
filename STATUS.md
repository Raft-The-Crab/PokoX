# STATUS.md — Verified Project State

## Current milestone

**Milestone:** Phase 0 — repository/build baseline inspection
**Current task:** `PKX-004` (READY)
**Project implementation status:** PKX-001, PKX-002, and PKX-003 (enhanced) completed. CMake, CTest, and PokoCore integration verified.

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
| PKX-003 Core primitives | DONE | Created PokoCore static library with Result/Error type, Logger with RAII, and EntityId (generational handles). Enhanced with: expanded error codes (40+ codes across 10 categories), error category system, stack trace support (placeholder), Logger file output with rotation, multiple log formats (Text/JSON/CSV), additional ID types (ResourceId, AssetId, TextureId, MeshId, AudioId), EntityPool for generational handle management. Source files organized in engine/core/src/error/, engine/core/src/logging/, engine/core/src/ids/. Public APIs have Doxygen comments. Unit tests in tests/unit/test_result.cpp, test_logger.cpp, test_entity_id.cpp, test_resource_id.cpp, test_entity_pool.cpp, test_main.cpp. Configure succeeded, build succeeded, CTest executed: `ctest --test-dir build/debug --output-on-failure` - CoreTest passed (0.03 sec). |
| PKX-004 Windows lifecycle | READY | PKX-003 complete |
| PKX-010+ Engine and product slices | BLOCKED | Dependency chain in `TASKS.md` |

## Latest verified change

- 2026-10-21: PKX-003 enhanced
- Files created: engine/core/include/poko/core/ResourceId.hpp, engine/core/include/poko/core/EntityPool.hpp, engine/core/src/ids/EntityPool.cpp, tests/unit/test_resource_id.cpp, tests/unit/test_entity_pool.cpp
- Files modified: engine/core/include/poko/core/Result.hpp (added ErrorCategory enum, expanded ErrorCode to 40+ codes, added StackFrame and stack trace support), engine/core/src/error/Result.cpp (updated error name mapping, added category formatting), engine/core/include/poko/core/Logger.hpp (added LogFormat enum, file output with rotation, multiple format support, async placeholder), engine/core/src/logging/Logger.cpp (implemented file output, JSON/CSV formatting, rotation placeholder), tests/unit/test_result.cpp (added category and new code tests), tests/unit/test_logger.cpp (added format tests), tests/unit/test_main.cpp (integrated new tests), engine/core/CMakeLists.txt (added new sources), tests/CMakeLists.txt (added new test files)
- Toolchain: Windows 11 x64, GCC 16.2.0 (MSYS2), CMake 4.4.2
- Configure command: `cmake -B build/debug -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=C:/msys64/ucrt64/bin/gcc.exe -DCMAKE_CXX_COMPILER=C:/msys64/ucrt64/bin/g++.exe`
- Build command: `cmake --build build/debug`
- Test command: `ctest --test-dir build/debug --output-on-failure`
- Result: Configure succeeded, build succeeded, CTest executed successfully (CoreTest passed in 0.03 sec)
- All acceptance criteria for PKX-003 satisfied, plus enhancements for error categories, stack trace support, Logger file output/rotation/formats, additional ID types, and EntityPool

## Update format

For each update, record:
- Date/time
- Task ID
- Files changed
- Exact commands executed
- Pass/fail result
- Remaining blocker
- Next task ID
