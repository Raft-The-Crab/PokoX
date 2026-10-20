---
id: "docs-decisions-open-decisions-md"
title: "Open Decisions Register"
kind: "implementation-contract"
execution_phase: "later-feature"
status: "specification"
source_of_truth: "This file plus ../02-architecture/module-boundaries.md and ../11-engineering/testing-and-ci.md"
---

# Agent Contract: Open Decisions Register

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

- **Owns:** Open Decisions Register
- **Document:** `docs/decisions/open-decisions.md`
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

- [ ] Implementation matches the requirements defined in `docs/decisions/open-decisions.md`.
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

These decisions should remain open until prototype evidence or a real production constraint makes a choice necessary.

| Decision | Current direction | What resolves it |
|---|---|---|
| Exact Diligent revision/backend | Diligent; Direct3D 11 is a candidate | Build and validate official sample on target toolchain |
| Math library | Small PokoX math layer or GLM | Compare actual needs and maintenance cost |
| ECS storage | PokoX-owned hybrid ECS; EnTT only if justified | Prototype hierarchy/component performance |
| Physics scope/details | Custom limited engine behind interface | Character-controller and trigger test suite |
| MUTE grammar | Lua-inspired, interpreter first | Formal grammar and conformance tests |
| MUTE type system | Optional checking modes proposed | Compiler prototype and creator feedback |
| MUTE host API | Handle-based capability API | Vertical slice and security review |
| `.pkxs` / `.pkx` schemas | Proposed extensions | Round-trip, migration, package validation prototype |
| Backend provider | Worker/D1/R2 or managed PostgreSQL alternative | Current pricing, quotas, terms, region, ops review |
| Identity provider | Firebase Auth candidate | Platform support, age/region, terms and security review |
| Payment provider | Not selected | Regional availability, legal/tax/payout and fee review |
| Multiplayer transport | WebSocket prototype or later UDP-based transport | Measure game requirements and latency |
| Multiplayer hosting | Not selected | Cost, scaling, authority, operations, regional needs |
| Android minimum/device matrix | Android 11+ proposal | Test hardware matrix and current toolchain |
| UI text shaping/fonts | Reuse mature infrastructure | Requirements, licensing, platform tests |
| Poko Mod scope | Intentionally open | Core workflows and creator feedback |
| Pokoins | Deferred | Ledger, atomicity, abuse, policy review |
| Open chat | Deferred | Moderation capacity and child-safety readiness |

## Decision process

When making a durable architecture choice:
1. Write a short ADR.
2. State the context and constraints.
3. List realistic alternatives.
4. Record the decision and tradeoffs.
5. Define how it can be revisited.
6. Link implementation and tests.
