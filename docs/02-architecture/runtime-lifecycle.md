---
id: "docs-02-architecture-runtime-lifecycle-md"
title: "Runtime Lifecycle and Frame Loop"
kind: "implementation-contract"
execution_phase: "architecture-foundation"
status: "specification"
source_of_truth: "This file plus ../02-architecture/module-boundaries.md and ../11-engineering/testing-and-ci.md"
---

# Agent Contract: Runtime Lifecycle and Frame Loop

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

- **Owns:** Runtime Lifecycle and Frame Loop
- **Document:** `docs/02-architecture/runtime-lifecycle.md`
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

- [ ] Implementation matches the requirements defined in `docs/02-architecture/runtime-lifecycle.md`.
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

## Startup

1. Parse command-line/project/package arguments.
2. Initialize diagnostics.
3. Initialize platform window and input.
4. Initialize renderer.
5. Load and validate project/package metadata.
6. Instantiate scene and components.
7. Initialize gameplay and MUTE within configured capabilities.
8. Enter loop only after required stages succeed.

Initialization should unwind safely in reverse order if any step fails.

## Frame loop target

A typical order:
1. Poll platform events and input.
2. Update input contexts and named actions.
3. Run variable-step gameplay updates.
4. Run fixed-step physics with a bounded catch-up count.
5. Synchronize physics/world state.
6. Deliver queued events at safe points.
7. Create a render snapshot.
8. Render and present.
9. Record diagnostics and timing.

Initial physics target: 60 Hz, with bounded catch-up. This does not guarantee deterministic simulation.

## Shutdown

- Stop accepting new work.
- Cancel or finish bounded tasks safely.
- Stop gameplay callbacks.
- Wait for renderer GPU work as required.
- Release runtime resources in dependency-safe order.
- Destroy platform window last among window-dependent systems.
- Flush diagnostics.

## Time and focus

- Simulation time and wall-clock time must be distinct.
- Long frames must not cause unbounded physics catch-up.
- Pause/resume behavior must be explicit.
- UI text entry and modal UI should prevent gameplay actions from leaking through.
- Runtime frame pacing should be profiled on reference hardware rather than assumed.
