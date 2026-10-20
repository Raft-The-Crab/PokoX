---
id: "docs-01-product-mvp-and-acceptance-md"
title: "MVP and Acceptance Criteria"
kind: "implementation-contract"
execution_phase: "product-contract"
status: "specification"
source_of_truth: "This file plus ../02-architecture/module-boundaries.md and ../11-engineering/testing-and-ci.md"
---

# Agent Contract: MVP and Acceptance Criteria

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

- **Owns:** MVP and Acceptance Criteria
- **Document:** `docs/01-product/mvp-and-acceptance.md`
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

- [ ] Clean checkout builds using documented instructions.
- [ ] Debug and Release configurations build.
- [ ] Window opens, resizes, and closes safely.
- [ ] Renderer initializes and shuts down correctly, including partial-init failures.
- [ ] A basic 3D scene renders.
- [ ] Scene transforms and component data save/reload.
- [ ] UI Maker can create, edit, save, reopen, and preview a screen.
- [ ] UI button has a working interaction.
- [ ] MUTE script executes within configured limits.
- [ ] Package validator catches invalid paths, missing dependencies, invalid hashes, and unsupported versions.
- [ ] Validator does not execute scripts.
- [ ] Player runs the built package without Studio.
- [ ] Tests run through CTest or the documented test runner.
- [ ] Public APIs introduced in the milestone have Doxygen comments.
- [ ] README contains exact build and test commands.
- [ ] No secrets or personal data are written to logs.

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

## MVP statement

A creator can create a project, build a simple 3D scene, add basic gameplay and a UI Maker screen, attach a MUTE script, save and reopen the project, build a validated runtime package, and play it in PokoX Player on Windows.

## MVP workflow

1. Launch Poko Studio.
2. Create a project from a blank project or simple template.
3. Create a scene containing a floor, platforms, camera, light, and player spawn.
4. Add a character controller and static colliders.
5. Add one hazard, checkpoint, collectible, and finish trigger.
6. Build a menu/HUD in UI Maker.
7. Attach a MUTE script to handle one or more gameplay events.
8. Save the project and close Studio.
9. Reopen the project and verify that the scene and UI are intact.
10. Build a `.pkx` runtime package.
11. Run package validation without executing scripts.
12. Launch the package in Player.
13. Play, quit, relaunch, and verify the experience is reproducible.

## Acceptance checklist

- [ ] Clean checkout builds using documented instructions.
- [ ] Debug and Release configurations build.
- [ ] Window opens, resizes, and closes safely.
- [ ] Renderer initializes and shuts down correctly, including partial-init failures.
- [ ] A basic 3D scene renders.
- [ ] Scene transforms and component data save/reload.
- [ ] UI Maker can create, edit, save, reopen, and preview a screen.
- [ ] UI button has a working interaction.
- [ ] MUTE script executes within configured limits.
- [ ] Package validator catches invalid paths, missing dependencies, invalid hashes, and unsupported versions.
- [ ] Validator does not execute scripts.
- [ ] Player runs the built package without Studio.
- [ ] Tests run through CTest or the documented test runner.
- [ ] Public APIs introduced in the milestone have Doxygen comments.
- [ ] README contains exact build and test commands.
- [ ] No secrets or personal data are written to logs.

## Definition of done for a feature

A feature is not done just because a screen or API exists. It should have:
- Clear owner and interface
- Input validation and defined failure behavior
- Tests appropriate to its risk
- Documentation for public use
- Integration with the actual workflow
- Resource and performance limits where relevant
- A clear statement of known limitations
