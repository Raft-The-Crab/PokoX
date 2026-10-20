---
id: "docs-04-studio-ui-maker-md"
title: "Poko Studio UI Maker — Flagship Feature"
kind: "implementation-contract"
execution_phase: "later-feature"
status: "specification"
source_of_truth: "This file plus ../02-architecture/module-boundaries.md and ../11-engineering/testing-and-ci.md"
---

# Agent Contract: Poko Studio UI Maker — Flagship Feature

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

- **Owns:** Poko Studio UI Maker — Flagship Feature
- **Document:** `docs/04-studio/ui-maker.md`
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

- [ ] Implementation matches the requirements defined in `docs/04-studio/ui-maker.md`.
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

## Product priority

The UI Maker is a major differentiator for PokoX. It should be exceptionally approachable for beginners while still allowing advanced creators to access precise controls. It is not an afterthought.

## Chosen approach

A **hybrid editor**: beginner-friendly defaults, with advanced controls available on demand. One document model and one editor, not separate beginner and expert products.

## Layout

- Left: UI hierarchy
- Center: canvas and preview
- Right: properties
- Bottom: UI library/components/assets
- Top: device preview, screen size, undo/redo, Play/Preview

## Initial elements

- Panel/container
- Text
- Button
- Image
- Progress bar
- Checkbox/toggle
- Slider
- Text input
- Spacer
- Horizontal/vertical/grid container
- Scroll container

Later: tabs, dropdowns, radial menus, virtualized lists, richer data-bound widgets.

## Layout system

Support:
- Position and size
- Anchors and offsets
- Relative sizing
- Min/max dimensions
- Horizontal, vertical, and grid containers
- Padding and spacing
- Alignment
- Responsive constraints
- Preview sizes/aspect ratios for desktop and mobile

The same layout/measurement engine must be used in the editor and runtime to minimize preview/runtime differences.

## Editing interactions

- Select, drag, resize
- Snapping and alignment guides
- Hierarchy selection for overlapping elements
- Duplicate, delete, rename, group
- Undo/redo
- Copy/paste
- Clear hierarchy and selection state
- Stable element IDs

## Reusable design

- Reusable components with exposed properties
- Templates: HUD, main menu, inventory, settings
- Themes and variants
- Style inheritance where it remains understandable
- Asset references that survive project moves

## Animation

Initial:
- Fade
- Position/scale
- Color/opacity
- Hover/pressed states
- Easing, duration, delay

Advanced timeline editing is deferred.

## Interaction model

Built-in interactions should cover common tasks:
- Button opens/closes/toggles a panel
- Button invokes a named action
- Toggle/slider changes a value
- Screen transition
- Hover/pressed visual states

Use MUTE for advanced behavior. Exact API names are illustrative until the runtime and language are implemented.

UI focus rules must prevent gameplay input from leaking through while a text input or modal UI is active.

## Technical split

- UI document model
- Layout and measurement engine
- Editor tools
- Runtime renderer/integration
- Shared assets
- Serialization/versioning

Use PokoX's renderer rather than creating a separate graphics stack. Reuse mature text shaping/font infrastructure when necessary; do not reinvent complex typography without a strong reason.

## First UI Maker acceptance test

Create a screen with a panel, text, and button; arrange and edit it; save and reopen; preview it at multiple sizes; run it in Player; verify the button works and layout remains usable.
