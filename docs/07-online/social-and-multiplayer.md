---
id: "docs-07-online-social-and-multiplayer-md"
title: "Social and Multiplayer"
kind: "implementation-contract"
execution_phase: "deferred-online"
status: "specification"
source_of_truth: "This file plus ../02-architecture/module-boundaries.md and ../11-engineering/testing-and-ci.md"
---

# Agent Contract: Social and Multiplayer

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

- **Owns:** Social and Multiplayer
- **Document:** `docs/07-online/social-and-multiplayer.md`
- **Execution phase:** `deferred-online`
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

- [ ] Implementation matches the requirements defined in `docs/07-online/social-and-multiplayer.md`.
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

## Social scope

Initial online social features, only after moderation readiness:
- Profiles and privacy settings
- Friend requests
- Block/report
- Presence
- Invitations
- Notification inbox

Later:
- Parties/groups
- Chat
- More complex social graphs

Open chat is deferred until moderation operations and child-safety protections are ready.

Blocking should apply consistently to invitations, messages, and presence visibility. Presence expires. All mutations require rate limits and clear privacy/retention rules.

## Multiplayer session lifecycle

1. Client requests a session.
2. Backend authenticates and authorizes.
3. Backend selects a compatible host/server.
4. Client receives short-lived session credentials.
5. Client and server negotiate protocol/schema versions.
6. Server validates inputs and owns authoritative state.
7. Client receives snapshots/events.
8. Disconnect/reconnect and expiry are handled explicitly.

## Transport

A WebSocket-based reliable transport may be useful for an early prototype. A UDP-based transport may be needed later for latency-sensitive gameplay. This is an open decision; do not build a custom transport or cryptography before measuring actual needs.

## Message families

- Handshake
- Input with sequence numbers
- State snapshot
- Reliable event
- RPC request/response
- Heartbeat
- Disconnect/reason

All payloads must be bounded and schema-validated.

## Server authority

- Clients submit intent/input.
- Server validates state changes.
- Purchases, inventory, rewards, match outcomes, and permissions are never trusted solely because the client claims them.
- Replication is explicit; components are not automatically replicated by default.
- Offline games should work offline when the game design permits it.
