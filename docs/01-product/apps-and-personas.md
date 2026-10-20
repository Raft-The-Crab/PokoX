---
id: "docs-01-product-apps-and-personas-md"
title: "Apps and User Workflows"
kind: "implementation-contract"
execution_phase: "product-contract"
status: "specification"
source_of_truth: "This file plus ../02-architecture/module-boundaries.md and ../11-engineering/testing-and-ci.md"
---

# Agent Contract: Apps and User Workflows

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

- **Owns:** Apps and User Workflows
- **Document:** `docs/01-product/apps-and-personas.md`
- **Execution phase:** `product-contract`
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

- [ ] Implementation matches the requirements defined in `docs/01-product/apps-and-personas.md`.
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

## PokoX Player

Initial targets: Windows 11 and later Android 11+ target, subject to actual device compatibility testing.

Player responsibilities:
- Browse or open a game package.
- Verify package compatibility before loading.
- Download content through bounded, resumable workflows where supported.
- Display clear errors for unsupported, incompatible, missing, corrupt, or resource-heavy content.
- Launch, pause/resume where applicable, quit, and relaunch.
- Keep app updates separate from content updates.
- Never trust client state for entitlements or multiplayer authority.

## Poko Studio

Windows 11 first. Android Studio is deferred until desktop workflows are proven.

Responsibilities:
- Welcome/recent projects/templates
- Project and scene management
- World Editor
- UI Maker
- Asset Browser and import
- MUTE editing and validation
- Play mode and Output panel
- Package build and validation
- Publishing later, after online backend and safety systems are ready

## Poko Mod

Windows 11 initial target. Its exact MVP scope is intentionally open. It must not become a blocker for the first Studio/Player vertical slice.

## Website

Later website responsibilities:
- Game discovery and detail pages
- Creator profiles and published version information
- Documentation and tutorials
- Reports, appeals, and support entry points
- Creator dashboard and publishing status
- Marketplace listings when commerce is ready

## Main personas

- **New creator:** wants to make something playable with minimal setup.
- **Advanced creator:** needs reusable components, scripting, asset management, debugging, and precise controls.
- **Player:** wants reliable downloads, understandable compatibility, and simple launch behavior.
- **Asset creator:** wants clear licensing, versioning, attribution, and distribution.
- **Moderator/support operator:** needs review queues, audit trails, and safe tools.
- **Platform operator:** needs observability, backups, incident response, and controlled releases.
