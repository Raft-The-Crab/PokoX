---
id: "docs-02-architecture-overview-md"
title: "Architecture Overview"
kind: "implementation-contract"
execution_phase: "architecture-foundation"
status: "specification"
source_of_truth: "This file plus ../02-architecture/module-boundaries.md and ../11-engineering/testing-and-ci.md"
---

# Agent Contract: Architecture Overview

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

- **Owns:** Architecture Overview
- **Document:** `docs/02-architecture/overview.md`
- **Execution phase:** `architecture-foundation`
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

- [ ] Implementation matches the requirements defined in `docs/02-architecture/overview.md`.
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

## Design shape

Use a modular monolith for backend application logic initially. Keep the engine split into narrow C++ modules. Do not begin with microservices.

## Major layers

- Apps: Player, Studio, Mod, website
- Runtime orchestration
- Core/platform
- Rendering, world, physics, assets, audio
- MUTE and gameplay host APIs
- Project/package pipeline
- Online backend and external providers

## Proposed engine modules

- `PokoCore`: foundational types, diagnostics, errors, utilities.
- `PokoPlatform`: operating-system window, input, lifecycle, file boundary.
- `PokoRender`: renderer interface and Diligent implementation.
- `PokoWorld`: entities, components, transforms, hierarchy, scene data.
- `PokoPhysics`: custom basic physics behind `IPhysicsWorld`.
- `PokoAssets`: asset IDs, import metadata, runtime resource loading.
- `PokoAudio`: audio abstraction; miniaudio is a candidate when audio begins.
- `PokoMUTE`: lexer/parser/runtime/sandbox/host boundary.
- `PokoGameplay`: input actions, lifecycle, events, interaction components.
- `PokoPackage`: project/runtime package formats and validation.
- `PokoNetwork`: protocol schemas, session transport integration.
- `PokoDiagnostics`: profiling, logs, crash context.
- `PokoInterop`: future C ABI for Rust/Kotlin.
- `PokoRuntime`: coordinates lifecycle and subsystems.

Only implement modules when the current milestone needs them. The full list is a destination architecture, not a requirement to create empty abstractions everywhere on day one.

## Dependency rules

- Core has no dependency on other PokoX modules.
- World does not depend on Diligent or MUTE implementation details.
- Physics does not depend on renderer types or MUTE internals.
- Only PokoRender owns Diligent types.
- Runtime coordinates modules; it should not become a giant home for subsystem logic.
- UI Maker document/layout/runtime are PokoX-owned and share one document model.
- Platform-specific gameplay logic is forbidden.
- MUTE receives handles and narrow host APIs, never raw engine pointers.
- Online/commerce authorization belongs to backend services.

## Replaceable interfaces

Keep replaceable:
- Renderer behind `IRenderer`
- Physics behind `IPhysicsWorld`
- Audio behind `PokoAudio`
- Platform lifecycle behind a platform adapter
- Host APIs behind MUTE capabilities
- Backend providers behind application-owned interfaces where a realistic alternate provider exists

Avoid abstraction for its own sake. Add interfaces where they isolate real change, testing, platform differences, or trust boundaries.
