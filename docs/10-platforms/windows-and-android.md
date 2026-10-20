---
id: "docs-10-platforms-windows-and-android-md"
title: "Windows and Android Platform Plan"
kind: "implementation-contract"
execution_phase: "later-feature"
status: "specification"
source_of_truth: "This file plus ../02-architecture/module-boundaries.md and ../11-engineering/testing-and-ci.md"
---

# Agent Contract: Windows and Android Platform Plan

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

- **Owns:** Windows and Android Platform Plan
- **Document:** `docs/10-platforms/windows-and-android.md`
- **Execution phase:** `later-feature`
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

- [ ] Implementation matches the requirements defined in `docs/10-platforms/windows-and-android.md`.
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

## Windows first

- Windows 11 x64
- C++20 engine
- CMake
- Visual Studio 2022
- Native Win32 window/input initially
- Diligent renderer
- Windows Player first
- Studio desktop workflows first

The first milestone must be buildable and testable on a clean Windows checkout.

## Android

- Android 11+ is the initial target proposal; confirm actual supported device matrix before public launch.
- Kotlin and Jetpack Compose for Android UI.
- Shared C++ runtime through a stable C ABI.
- Diligent surface/lifecycle integration must handle pause/resume and surface recreation.
- Touch input maps to the same named input action model.
- Downloads are asynchronous and resumable where feasible.
- Handle app backgrounding, low memory, device rotation, and process recreation.
- Keep platform-specific code at the boundary.

## Interop

Do not expose C++ object layouts or raw pointers to Kotlin/Rust. Design explicit C-compatible handles, structs, versioned functions, error codes, and ownership rules. Keep ABI evolution intentional.

## Compatibility

Every package declares supported platform/runtime ranges. Player should distinguish:
- Unsupported platform
- Incompatible runtime version
- Missing dependency
- Corrupt package
- Resource limit exceeded
- Temporary network/service error

Android Studio is a later phase after Windows workflow is stable.
