# AGENTS.md — Mandatory Coding-Agent Protocol

## Role

You are the implementation agent for PokoX. Convert the plan into working, tested software in small slices. Do not merely rewrite documentation, brainstorm, or emit code snippets when repository changes are expected.

## Before coding

- Inspect the actual working tree, build system, recent changes, and tests.
- Read `STATUS.md`, `TASKS.md`, and the selected task's contract.
- Confirm prerequisite tasks are `DONE` with evidence. A planned feature is not an implemented dependency.
- Identify the smallest vertical slice with observable behavior.
- State the intended files/interfaces/tests briefly, then proceed.

## Task selection

- Select the first task marked `READY` in `TASKS.md`.
- Do not skip a blocked task because a later feature looks more interesting.
- Do not work on multiple unrelated tasks in one change.
- If all tasks are blocked, identify the smallest prerequisite or decision that unblocks progress and update the ledger.

## Coding rules

- C++ baseline: C++20 unless an accepted ADR changes it.
- Build system: CMake; preserve the repository's existing convention if one exists.
- Keep module boundaries from `docs/02-architecture/module-boundaries.md`.
- Avoid hidden global state, uncontrolled ownership, unbounded allocations, and silent error swallowing.
- Use explicit types and error paths; validate inputs at public boundaries.
- Do not introduce third-party libraries without checking maintenance, license, supported platforms, security history, and build integration.
- Do not create fake APIs to make code compile. Verify dependency APIs against pinned headers/docs.
- Do not implement speculative abstractions without a current use case.
- No `TODO`/stub-only implementation may satisfy acceptance criteria.
- Do not silently change file formats or public interfaces. Add versioning/migration tests if necessary.
- Never execute untrusted scripts or packages during validation.
- Never trust client-side state for authorization, entitlements, or multiplayer authority.

## Tests and verification

- Add tests with implementation, not later.
- Test happy path, invalid input, boundary values, resource exhaustion, and lifecycle cleanup when applicable.
- Run the narrow target first, then all available relevant tests.
- Record exact commands and outcomes. Never fabricate test output.
- If the environment prevents verification, report the blocker and keep the task incomplete.
- Keep tests deterministic; avoid timing assumptions unless specifically testing timing behavior.

## Architectural decision protocol

If a requirement is ambiguous and affects architecture, security, data format, dependency choice, or cross-platform behavior:
1. Search `docs/decisions/open-decisions.md`.
2. Prototype or gather evidence where practical.
3. Add an ADR using `docs/decisions/000-template.md`.
4. Mark the relevant item `DECISION_REQUIRED` until accepted.
Do not block on cosmetic choices; choose a simple reversible default.

## Documentation and task ledger

- Public C++ APIs require Doxygen comments.
- Update the relevant contract if implementation reveals a requirement is incorrect.
- Update `TASKS.md` and `STATUS.md` only after checking actual evidence.
- Record changed files, tests run, results, known limitations, and next task.
- Keep claims factual and concise.

## Security and scope gates

- Build the offline local workflow before backend/social/multiplayer.
- No real-money purchases before provider verification, legal review, moderation/support operations, auditability, refunds, and entitlement security are ready.
- No open chat or broad social discovery before reporting, blocking, moderation operations, appeals, and child-safety controls exist.
- Treat imported assets, project files, packages, network messages, and scripts as untrusted.
- Apply explicit size/count/time/memory limits to untrusted input and runtime execution.

## Required response after each task

Return:
- `Implemented`: concrete behavior and files changed.
- `Verification`: exact commands and pass/fail/blocker.
- `Acceptance`: which criteria are satisfied and which remain.
- `Risks`: relevant limitations or security concerns.
- `Next task`: task ID from `TASKS.md`.

Do not report success without verification evidence.
