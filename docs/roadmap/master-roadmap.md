---
id: "docs-roadmap-master-roadmap-md"
title: "Master Roadmap"
kind: "implementation-contract"
execution_phase: "roadmap"
status: "specification"
source_of_truth: "This file plus ../02-architecture/module-boundaries.md and ../11-engineering/testing-and-ci.md"
---

# Agent Contract: Master Roadmap

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

- **Owns:** Master Roadmap
- **Document:** `docs/roadmap/master-roadmap.md`
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

- [ ] Implementation matches the requirements defined in `docs/roadmap/master-roadmap.md`.
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

The roadmap is ordered by dependencies and risk. It is not a promise of dates.

## Phase 0 — Contracts and repo

- Freeze initial repo layout and CMake convention.
- Add core logging/errors/IDs.
- Add Win32 window lifecycle.
- Add Doxygen and formatting.
- Build and test from clean checkout.
- Pin Diligent revision.

## Phase 1 — Engine foundation

- Integrate Diligent in PokoRender.
- Clear/present, resize, shutdown.
- Render triangle/cube and basic camera.
- Establish renderer resource ownership.
- Add minimal diagnostics and tests.

## Phase 2 — World and basic gameplay

- Entity/component model and hierarchy.
- Transform serialization.
- Basic meshes/materials/camera/light.
- Basic custom physics and character controller.
- Input action contexts.
- Play mode using a runtime scene copy.

## Phase 3 — MUTE and content formats

- Finalize grammar and semantics.
- Implement lexer/parser/interpreter.
- Implement budgets/capabilities.
- Define `.pkxs` project and `.pkx` package schemas.
- Implement non-executing package validator.
- Add package build report and tests.

## Phase 4 — Studio and UI Maker

- Welcome/recent projects.
- World Editor selection/transforms/hierarchy/Inspector.
- Save/reopen and undo/redo.
- UI Maker document/layout/editor/runtime.
- Button interactions and responsive preview.
- Asset import and basic prefabs.

## Phase 5 — Player and local publishing

- Player launch and lifecycle.
- Package loading and clear compatibility errors.
- End-to-end obstacle-course vertical slice.
- Build/relaunch from a clean project.
- Document creator workflow.

## Phase 6 — Online foundation

- Select providers after terms/quotas/security review.
- Identity mapping and accounts.
- Catalog and content upload/publishing.
- Validation/review workflow.
- Creator dashboard basics.

## Phase 7 — Safety and multiplayer

- Reports, blocks, moderation queue, appeals.
- Multiplayer session lifecycle.
- Server-authoritative state and protocol schemas.
- Load/abuse tests.
- Open chat remains deferred until operational readiness.

## Phase 8 — Social and marketplace

- Add social features in safe increments.
- Start with free marketplace listings.
- Add real-money only after legal/provider/security/support readiness.
- Immutable ledger, entitlements, refunds, disputes, creator payouts.

## Phase 9 — Android

- C ABI stability and Android lifecycle integration.
- Kotlin/Compose app shell.
- Touch input mapping.
- Surface recreation, pause/resume, memory and download behavior.
- Reference-device performance and compatibility matrix.

## Phase 10 — Expansion

- Terrain, skeletal animation, navigation, richer effects.
- Advanced UI components and animations.
- Larger content ecosystem.
- More transport options only when justified by profiling.

## Rule for every phase

Do not begin the next phase just because the calendar says so. Complete acceptance criteria, fix critical defects, document limitations, and preserve a working build.
