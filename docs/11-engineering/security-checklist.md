---
id: "docs-11-engineering-security-checklist-md"
title: "Engineering Security Checklist"
kind: "implementation-contract"
execution_phase: "engineering-quality"
status: "specification"
source_of_truth: "This file plus ../02-architecture/module-boundaries.md and ../11-engineering/testing-and-ci.md"
---

# Agent Contract: Engineering Security Checklist

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

- **Owns:** Engineering Security Checklist
- **Document:** `docs/11-engineering/security-checklist.md`
- **Execution phase:** `engineering-quality`
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

- [ ] Reject path traversal and unsafe archive entries.
- [ ] Enforce compressed and decompressed size limits.
- [ ] Validate hashes and schema versions.
- [ ] Bound collection counts and asset dimensions.
- [ ] Validate dependencies and capabilities.
- [ ] Never execute scripts during validation.
- [ ] Reject unknown security-sensitive fields.
- [ ] Keep published versions immutable.
- [ ] No unrestricted filesystem/OS/network/native access.
- [ ] Enforce instruction, memory, stack, task, and callback budgets.
- [ ] Capability-check host APIs.
- [ ] Bound import depth and dependencies.
- [ ] Test budget exhaustion and permission denial.
- [ ] Avoid leaking secrets in diagnostics.
- [ ] Authenticate and authorize every mutation.
- [ ] Use idempotency for retryable financial/state changes.
- [ ] Rate-limit sensitive endpoints.
- [ ] Validate all client inputs server-side.
- [ ] Keep server authority for rewards, inventory, purchases, and match outcomes.
- [ ] Use short-lived upload/session credentials.
- [ ] Do not implement custom cryptography.
- [ ] Audit privileged administrative actions.
- [ ] Verify signed provider events.
- [ ] Process webhooks idempotently.
- [ ] Reconcile provider records and internal ledger.
- [ ] Record refunds/disputes and entitlement changes.
- [ ] Protect creator payout actions.
- [ ] Keep money in integer minor units and explicit currency.
- [ ] Backups and restore drills.
- [ ] Quota/usage alerts.
- [ ] Incident playbooks.
- [ ] Key rotation and access controls.
- [ ] Dependency/license tracking.
- [ ] Privacy and retention rules.

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

## Packages and assets

- [ ] Reject path traversal and unsafe archive entries.
- [ ] Enforce compressed and decompressed size limits.
- [ ] Validate hashes and schema versions.
- [ ] Bound collection counts and asset dimensions.
- [ ] Validate dependencies and capabilities.
- [ ] Never execute scripts during validation.
- [ ] Reject unknown security-sensitive fields.
- [ ] Keep published versions immutable.

## MUTE

- [ ] No unrestricted filesystem/OS/network/native access.
- [ ] Enforce instruction, memory, stack, task, and callback budgets.
- [ ] Capability-check host APIs.
- [ ] Bound import depth and dependencies.
- [ ] Test budget exhaustion and permission denial.
- [ ] Avoid leaking secrets in diagnostics.

## Backend and multiplayer

- [ ] Authenticate and authorize every mutation.
- [ ] Use idempotency for retryable financial/state changes.
- [ ] Rate-limit sensitive endpoints.
- [ ] Validate all client inputs server-side.
- [ ] Keep server authority for rewards, inventory, purchases, and match outcomes.
- [ ] Use short-lived upload/session credentials.
- [ ] Do not implement custom cryptography.
- [ ] Audit privileged administrative actions.

## Commerce

- [ ] Verify signed provider events.
- [ ] Process webhooks idempotently.
- [ ] Reconcile provider records and internal ledger.
- [ ] Record refunds/disputes and entitlement changes.
- [ ] Protect creator payout actions.
- [ ] Keep money in integer minor units and explicit currency.

## Operations

- [ ] Backups and restore drills.
- [ ] Quota/usage alerts.
- [ ] Incident playbooks.
- [ ] Key rotation and access controls.
- [ ] Dependency/license tracking.
- [ ] Privacy and retention rules.
