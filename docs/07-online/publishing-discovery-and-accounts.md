---
id: "docs-07-online-publishing-discovery-and-accounts-md"
title: "Accounts, Publishing, and Discovery"
kind: "implementation-contract"
execution_phase: "deferred-online"
status: "specification"
source_of_truth: "This file plus ../02-architecture/module-boundaries.md and ../11-engineering/testing-and-ci.md"
---

# Agent Contract: Accounts, Publishing, and Discovery

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

- **Owns:** Accounts, Publishing, and Discovery
- **Document:** `docs/07-online/publishing-discovery-and-accounts.md`
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

- [ ] Implementation matches the requirements defined in `docs/07-online/publishing-discovery-and-accounts.md`.
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

## Accounts and roles

Keep identity separate from authorization. Candidate roles:
- Player
- Creator
- Moderator
- Support operator
- Administrator

Use least privilege, explicit permission checks, and auditable admin actions. Do not rely only on UI hiding.

## Publishing

A creator should see:
- Validation status
- Clear errors and remediation guidance
- Requested capabilities
- Dependency and license summary
- Supported platforms
- Immutable published version
- Rollback/unpublish controls with clear effects

## Discovery

Start with explainable, basic ranking signals:
- Relevance to search
- Quality and compatibility signals
- User-selected categories
- Freshness with safeguards
- Abuse/manipulation detection

Do not optimize only for clicks or play counts. Avoid opaque ranking until there is enough data and moderation capacity.

## Creator dashboard

Later dashboard:
- Projects and published versions
- Validation results
- Download/play metrics with privacy constraints
- Reports/status
- Marketplace sales/ledger when commerce launches
- Support and appeals

## Admin tooling

Must eventually include:
- User/content lookup with access controls
- Report/review queue
- Sanctions and appeals
- Publishing status
- Audit log
- Incident status
- Payment dispute/refund workflow when commerce launches
