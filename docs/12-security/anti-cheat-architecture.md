---
id: "docs-12-security-anti-cheat-architecture-md"
title: "PokoX Anti-Cheat & Security Architecture (v0.1)"
kind: "implementation-contract"
execution_phase: "security-gate"
status: "specification"
source_of_truth: "This file plus ../02-architecture/module-boundaries.md and ../11-engineering/testing-and-ci.md"
---

# Agent Contract: PokoX Anti-Cheat & Security Architecture (v0.1)

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

- **Owns:** PokoX Anti-Cheat & Security Architecture (v0.1)
- **Document:** `docs/12-security/anti-cheat-architecture.md`
- **Execution phase:** `security-gate`
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

- [ ] Implementation matches the requirements defined in `docs/12-security/anti-cheat-architecture.md`.
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

## Goals

PokoX is designed around **authoritative validation**, not invasive kernel drivers.

- No kernel anti-cheat.
- Cross-platform (Windows and Android).
- Client prediction with server validation.
- Local simulation remains responsive.
- Detect cheating through validation, replay, and heuristics.

## Threat model

Cheats considered:
- Speed hacks
- Fly/teleport hacks
- Memory editing
- Script injection
- Packet replay
- Fake RPCs
- Auto farming
- Exploit tools

Out of scope initially:
- Hardware cheats
- DMA attacks
- Kernel-level malware

## Trust boundaries

| Layer | Trust |
|---|---|
| Client | Low |
| Package | Verified |
| MUTE | Sandboxed |
| Worker/API | Trusted |
| Game session | Authoritative |
| Marketplace | Highly trusted |

## Multi-layer defense

1. Package validation before launch.
2. Sandboxed MUTE runtime.
3. Handle-based APIs only.
4. Bitstream schema validation.
5. Authoritative gameplay validation.
6. Behavior heuristics.
7. Audit logging.
8. Replay support.

## Server validation

The server validates:
- movement limits
- jump legality
- collision results
- damage
- rewards
- inventory
- purchases

Clients submit **intent**, not authority.

## Movement validation

Check:
- maximum speed
- acceleration
- gravity consistency
- ground state
- collision legality
- teleport authorization

## Replay support

Store:
- input timeline
- timestamps
- server decisions
- major events

Replays help investigate reports without trusting client recordings.

## MUTE security

- No raw pointers
- No filesystem
- No unrestricted networking
- No native DLL loading
- Resource budgets enforced

## Package integrity

Every package includes:
- manifest
- hashes
- dependency list
- capability declarations
- immutable version

Validation never executes scripts.

## Networking

- Schema-defined Bitstream
- Size limits
- Sequence numbers
- Replay protection where needed
- Rate limits

## Moderation integration

Suspicion feeds:
- reports
- heuristic flags
- repeated violations

Moderators see evidence instead of raw accusations.

## Release gates

Before public multiplayer:
- movement validation
- replay system
- audit logs
- rate limits
- package validation
- script budgets
- moderation tooling
