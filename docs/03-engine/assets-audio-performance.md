---
id: "docs-03-engine-assets-audio-performance-md"
title: "Assets, Audio, and Performance"
kind: "implementation-contract"
execution_phase: "engine-foundation"
status: "specification"
source_of_truth: "This file plus ../02-architecture/module-boundaries.md and ../11-engineering/testing-and-ci.md"
---

# Agent Contract: Assets, Audio, and Performance

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

- **Owns:** Assets, Audio, and Performance
- **Document:** `docs/03-engine/assets-audio-performance.md`
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

- [ ] Implementation matches the requirements defined in `docs/03-engine/assets-audio-performance.md`.
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

## Asset identity

Separate:
- Stable logical asset ID
- Content hash
- Source file path
- Importer version
- Generated runtime artifact
- Package/version reference

Asset paths inside a project should be relative/project-owned references, not absolute machine paths. Source files and generated runtime files are distinct.

## Initial formats

- Models: glTF/GLB
- Textures/UI images: PNG/JPEG initially
- Materials: PokoX-owned material definition
- Scripts: MUTE source
- Scenes/prefabs: PokoX schema
- Audio: select supported formats when audio is implemented, based on actual decoder needs and licensing

Do not add a large asset importer stack until model import is being implemented. Validate file size, structure, dimensions, decompression behavior, and supported features.

## Audio

Audio is planned behind a `PokoAudio` abstraction. `miniaudio` is a candidate when implementation begins, but is not a required Stage 1 dependency. Define device loss, unavailable device, decode failure, and resource limits.

## Performance

- Initial aim: 60 FPS on a defined reference PC (16.67 ms frame budget).
- Initial fixed physics target: 60 Hz with bounded catch-up.
- Profile before setting arbitrary entity limits.
- Track frame time, physics time, script time, draw/submit time, memory, and asset load times.
- Android needs its own reference devices and budgets.
- Script and package resource budgets must be explicit.
- Avoid unbounded loops, allocations, task queues, and catch-up steps.
