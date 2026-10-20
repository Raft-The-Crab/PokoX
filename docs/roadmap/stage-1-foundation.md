---
id: "docs-roadmap-stage-1-foundation-md"
title: "Stage 1 — Repository and Windows Build Foundation"
kind: "implementation-contract"
execution_phase: "roadmap"
status: "specification"
source_of_truth: "This file plus ../02-architecture/module-boundaries.md and ../11-engineering/testing-and-ci.md"
---

# Agent Contract: Stage 1 — Repository and Windows Build Foundation

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

- **Owns:** Stage 1 — Repository and Windows Build Foundation
- **Document:** `docs/roadmap/stage-1-foundation.md`
- **Execution phase:** `roadmap`
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

- [ ] Create repository and CMake presets.
- [ ] Add build options and warnings.
- [ ] Add Core logging and error result conventions.
- [ ] Implement Win32 window creation, resize, event pump, close.
- [ ] Ensure partial initialization unwinds safely.
- [ ] Pin Diligent and build its sample.
- [ ] Integrate Diligent only inside PokoRender.
- [ ] Render clear color, triangle, then cube.
- [ ] Handle resize/minimize/shutdown.
- [ ] Add minimal CTest tests.
- [ ] Add Doxygen config and public API comments.
- [ ] Add `.clang-format`.
- [ ] Add Windows CI.
- [ ] Document clean checkout build/test commands.
- [ ] Add ADRs for material decisions.

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

## Goal

Create a minimal C++20/CMake Windows application with clean lifecycle and a renderer integration path. Do not implement the whole engine at once.

## Repository

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
│   ├── CMakeLists.txt
│   └── src/main.cpp
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

## Environment

- Windows 11 x64
- Visual Studio 2022 with C++ Desktop workload
- Windows SDK
- CMake 3.24+ proposed baseline
- Git
- Doxygen and clang-format as optional developer tools

Exact minimum versions should be verified in the actual build.

## Dependency pinning

Diligent Engine is a submodule or otherwise pinned to a reviewed commit/tag. Build its official minimal example before integrating. Do not depend on the moving main branch.

## First module targets

- `PokoCore`
- `PokoPlatform`
- `PokoRender`
- `PokoRuntime`
- `PokoPlayerWindows`
- `PokoCoreTests`

## Stage tasks

- [ ] Create repository and CMake presets.
- [ ] Add build options and warnings.
- [ ] Add Core logging and error result conventions.
- [ ] Implement Win32 window creation, resize, event pump, close.
- [ ] Ensure partial initialization unwinds safely.
- [ ] Pin Diligent and build its sample.
- [ ] Integrate Diligent only inside PokoRender.
- [ ] Render clear color, triangle, then cube.
- [ ] Handle resize/minimize/shutdown.
- [ ] Add minimal CTest tests.
- [ ] Add Doxygen config and public API comments.
- [ ] Add `.clang-format`.
- [ ] Add Windows CI.
- [ ] Document clean checkout build/test commands.
- [ ] Add ADRs for material decisions.

## Definition of done

- Clean checkout builds Debug and Release.
- Test suite passes.
- Window opens, resizes, and closes.
- Renderer can initialize, render, and shut down.
- Failure during startup does not leak resources or crash.
- Diligent types remain confined to PokoRender.
- Public APIs are documented.
- README has exact tested commands.
