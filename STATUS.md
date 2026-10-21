# STATUS.md — Verified Project State

## Current milestone

**Milestone:** Phase 0 — repository/build baseline inspection
**Current task:** `PKX-003` (READY)
**Project implementation status:** PKX-001 and PKX-002 completed. CMake and CTest integration verified.

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
| PKX-003 Core primitives | READY | PKX-002 complete |
| PKX-004 Windows lifecycle | BLOCKED | Requires PKX-002 and PKX-003 |
| PKX-010+ Engine and product slices | BLOCKED | Dependency chain in `TASKS.md` |

## Latest verified change

- 2026-10-21: PKX-002 completed
- Files created: tests/unit/test_main.cpp
- Files modified: CMakeLists.txt (added CTest integration), tests/CMakeLists.txt (added test executable and registration), .github/workflows/ci.yml (pinned actions for security)
- Toolchain: Windows 11 x64, GCC 16.2.0 (MSYS2), CMake 4.4.2
- Configure command: `cmake -B build/debug -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=C:/msys64/ucrt64/bin/gcc.exe -DCMAKE_CXX_COMPILER=C:/msys64/ucrt64/bin/g++.exe`
- Build command: `cmake --build build/debug`
- Test command: `ctest --test-dir build/debug --output-on-failure`
- Result: Configure succeeded, build succeeded, CTest executed successfully (CoreTest passed in 0.25 sec)
- CI workflow created with pinned GitHub Actions (checkout@v4.2.2) for supply-chain security
- All acceptance criteria for PKX-002 satisfied

## Update format

For each update, record:
- Date/time
- Task ID
- Files changed
- Exact commands executed
- Pass/fail result
- Remaining blocker
- Next task ID
