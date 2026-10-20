---
id: "docs-03-engine-cpp-engine-foundation-md"
title: "C++ Engine Foundation"
kind: "implementation-contract"
execution_phase: "engine-foundation"
status: "specification"
source_of_truth: "This file plus ../02-architecture/module-boundaries.md and ../11-engineering/testing-and-ci.md"
---

# Agent Contract: C++ Engine Foundation

## Execution directive

Treat this document as an implementation contract, not as permission to implement the entire product in one pass. Work only on the smallest independently testable slice that is unblocked by the repository's current state.

1. Inspect the repository before editing. Reuse existing conventions and code; do not assume this plan describes code that already exists.
2. Identify the current milestone in `STATUS.md` and the next unchecked task in `TASKS.md`.
3. Read this contract and its linked dependency contracts before changing code.
4. Write a short implementation plan in the task response: files to touch, interfaces, tests, and risks. Then implement; do not stop at planning.
5. Keep changes narrowly scoped. Do not add speculative frameworks, placeholder implementations presented as complete, or unrelated features.
6. Build and run the narrowest relevant tests, then run the broader available test suite.
7. Update `STATUS.md` and `TASKS.md` only when the code and verification evidence justify the status change.
8. If a decision is unresolved, record the options and evidence in `docs/decisions/open-decisions.md`; do not silently invent a permanent architectural decision.
9. Never claim a build/test passed unless you actually ran it and observed success. If blocked by environment/tooling, report the exact blocker and the command not run.
10. Keep secrets, credentials, signing keys, private user data, and production endpoints out of source control.

## Scope

- **Owns:** C++ Engine Foundation
- **Document:** `docs/03-engine/cpp-engine-foundation.md`
- **Execution phase:** `engine-foundation`
- **Do not implement yet:** online/commerce/multiplayer/security-sensitive production features unless their prerequisite gates are met.

## Required implementation sequence

1. Establish current repository state and existing implementation.
2. Extract one concrete behavior or interface from the requirements below.
3. Define input/output contracts and error behavior before implementation.
4. Implement the minimum vertical slice.
5. Add tests before marking the slice complete.
6. Verify compilation, tests, and relevant platform constraints.
7. Update the task ledger with evidence and any remaining blocker.

## Acceptance contract

- [ ] Implementation matches the requirements defined in `docs/03-engine/cpp-engine-foundation.md`.
- [ ] Public interfaces and ownership boundaries are explicit.
- [ ] Failure cases are handled deterministically and produce actionable diagnostics.
- [ ] Automated tests cover normal behavior, invalid input, and boundary conditions.
- [ ] The change builds and tests on the currently supported target.

## Implementation constraints

- Prefer deterministic behavior and explicit ownership/lifetimes.
- Validate external, serialized, and user-controlled input at the boundary.
- Avoid unnecessary dependencies; pin any dependency that is introduced and document its license.
- Keep platform-specific code behind platform interfaces.
- Do not expose renderer, OS, networking, or scripting implementation details through unrelated modules.
- Add or update tests for every behavior change; include negative and boundary tests where applicable.
- Preserve backward compatibility deliberately. If a format/API changes, add a versioning or migration plan.
- A TODO is not acceptance evidence. A stub is not a completed feature.

## Requirements and design detail

The following requirements are preserved from the source plan. They are binding unless superseded by a recorded decision.

## Initial implementation choices

- C++20
- CMake 3.24+ as a starting baseline
- Windows 11 x64 first
- Visual Studio 2022 C++ Desktop workload
- Diligent Engine renderer
- Basic custom physics behind a replaceable interface
- Native Win32 window initially to minimize dependencies
- Rust/egui UI integration later, through a stable C ABI
- Kotlin/Jetpack Compose Android UI later, also through a stable C ABI

Confirm exact toolchain and Diligent revision compatibility during implementation.

## Initial repository shape

```text
pokox/
├── CMakeLists.txt
├── CMakePresets.json
├── .gitignore
├── .clang-format
├── Doxyfile
├── README.md
├── LICENSE
├── cmake/Dependencies.cmake
├── apps/player-windows/
├── engine/
│   ├── core/
│   ├── platform/
│   ├── render/
│   └── runtime/
├── tests/
├── third_party/README.md
├── docs/
└── .github/workflows/windows.yml
```

Create more engine modules as needed by a real milestone. Avoid creating many empty directories and speculative interfaces.

## Initial CMake target direction

- `PokoCore`: static library
- `PokoPlatform`: static library linked to Core
- `PokoRender`: static library linked to Core
- `PokoRuntime`: static library coordinating Core, Platform, Render
- `PokoPlayerWindows`: executable linked to Runtime
- `PokoCoreTests`: small test executable registered with CTest

Use an interface target such as `pokox_build_options` to centralize compiler settings. MSVC baseline may include `/W4`, `/permissive-`, `/Zc:__cplusplus`, and `/EHsc`; resolve warnings instead of suppressing them broadly.

## Initial CMake commands

```powershell
cmake --preset windows-debug
cmake --build --preset build-debug
ctest --preset test-debug
cmake --preset windows-release
cmake --build --preset build-release
```

Preset names and exact output paths must match the checked-in CMake files. These commands are a proposed convention, not an already-tested build.

## Renderer integration

Pin Diligent to a reviewed tag or commit. Build its smallest official example first, then integrate only the necessary backend/modules into `PokoRender`. Verify target names and options against the pinned revision. Direct3D 11 is a candidate initial Windows backend.

`PokoCore`, world data, and tests should remain buildable without the renderer dependency where practical.
