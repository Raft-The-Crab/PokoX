---
id: "docs-01-product-vision-and-scope-md"
title: "Vision and Scope"
kind: "implementation-contract"
execution_phase: "product-contract"
status: "specification"
source_of_truth: "This file plus ../02-architecture/module-boundaries.md and ../11-engineering/testing-and-ci.md"
---

# Agent Contract: Vision and Scope

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

- **Owns:** Vision and Scope
- **Document:** `docs/01-product/vision-and-scope.md`
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

- [ ] Implementation matches the requirements defined in `docs/01-product/vision-and-scope.md`.
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

## Vision

PokoX is a free-to-use user-generated-content game platform where people can play, create, publish, share, and eventually monetize games, reusable assets, plugins, and MUTE libraries.

The platform should feel approachable to beginners while allowing advanced creators to grow into more capable tools. PokoX should own its core product experience and important runtime systems without trying to reinvent every mature infrastructure component.

## Product surfaces

- **PokoX Player:** discovers, installs/downloads, launches, plays, and manages compatible games.
- **Poko Studio:** creates projects, scenes, gameplay, interfaces, assets, tests, and builds packages.
- **Poko Mod:** Windows tool for modding/workflow needs; exact scope to be defined after core workflows work.
- **Website:** discovery, creator pages, game pages, documentation, safety/reporting, and creator dashboard.
- **Backend:** identity mapping, catalog, content storage, publishing, sessions, social, safety, and commerce.
- **Marketplace:** free and, when safe and operationally ready, paid assets/plugins/MUTE libraries.

## Product principles

- Free access is the default.
- Creators should be able to make and test a local game before needing online services.
- The first experience should avoid unnecessary setup and dependencies.
- The UI Maker is a flagship capability, not a minor editor panel.
- A creator project and a published package are different artifacts.
- Published versions are immutable.
- The server owns important multiplayer and financial decisions.
- Safety and child protection must be designed before open social features.
- Platform features should be added based on working evidence rather than speculation.

## Ownership and dependencies

PokoX owns its application logic, formats, UI Maker, MUTE, world model, basic physics, package validation, and creator workflows. Third-party libraries remain under their own licenses. PokoX must preserve notices and license obligations.

Use mature external infrastructure for tasks where custom implementation would be risky or wasteful, especially payments, cryptography, identity, and storage. Do not write custom cryptography.

## Non-goals for the first milestone

- Public multiplayer
- Open text/voice chat
- Real-money transactions
- Full rigid-body physics, joints, ragdolls, or vehicles
- Large-world streaming
- Advanced terrain tooling
- Full animation graph and IK system
- Android Studio
- A large plugin ecosystem
- Microservices

These may be valuable later, but must not delay the first complete local creator-to-player workflow.
