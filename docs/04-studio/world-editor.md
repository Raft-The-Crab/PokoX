---
id: "docs-04-studio-world-editor-md"
title: "Poko Studio World Editor"
kind: "implementation-contract"
execution_phase: "later-feature"
status: "specification"
source_of_truth: "This file plus ../02-architecture/module-boundaries.md and ../11-engineering/testing-and-ci.md"
---

# Agent Contract: Poko Studio World Editor

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

- **Owns:** Poko Studio World Editor
- **Document:** `docs/04-studio/world-editor.md`
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

- [ ] Implementation matches the requirements defined in `docs/04-studio/world-editor.md`.
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

## Scope v0.1

- Hierarchy and parent-child relationships
- Object selection and manipulation
- Transform editing
- Basic meshes/materials/lights/cameras
- Static colliders, triggers, raycasts, character movement
- MUTE scripts and input actions
- Interactions
- Save/load, prefabs, Play mode

## Add Object menu

### Primitives
- Cube
- Sphere
- Plane
- Cylinder
- Capsule
- Empty group

### Gameplay objects
- Spawn point
- Camera
- Light
- Trigger
- Static collider
- Character controller
- Audio source when audio is available

### Assets
- Models
- Materials
- Textures
- Prefabs
- MUTE scripts

Place objects near viewport focus or on a selected surface when possible, not always at the world origin.

## Transform tools

- Move, rotate, scale
- World/local modes
- Grid, angle, and scale snapping
- Numeric transform fields
- Duplicate, delete, group, reparent
- Multi-select
- Focus selected
- Undo/redo
- Shortcuts W/E/R and F, while not hijacking text entry

## Viewport

- Orbit, pan, zoom
- Perspective/orthographic
- Front/back/left/right/top/bottom
- Focus selection and frame all
- Grid/gizmo/lighting toggles
- Mouse selection
- Active game camera indicator
- Editor camera separate from game camera

## Inspector

Common fields:
- Name
- Active state
- Transform
- Components
- Editor visibility vs game visibility
- Optional tags

Use a context-sensitive Add Component menu. Component metadata should drive Inspector fields and serialization validation to avoid duplicate definitions.

## Materials and lighting

Initial material controls:
- Base color
- Texture
- Roughness/metallic where supported
- Basic transparency
- Assign to mesh
- Presets

Initial lights:
- Directional
- Point
- Spot
- Ambient/environment support as renderer allows

Advanced GI/postprocessing are deferred.

## Play mode

The editor scene remains intact while a runtime copy executes. On Stop, restore edit state. Runtime changes do not persist unless the creator explicitly applies a supported workflow.
