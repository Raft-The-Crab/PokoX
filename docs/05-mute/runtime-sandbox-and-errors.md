---
id: "docs-05-mute-runtime-sandbox-and-errors-md"
title: "MUTE Runtime, Sandbox, and Diagnostics"
kind: "implementation-contract"
execution_phase: "later-feature"
status: "specification"
source_of_truth: "This file plus ../02-architecture/module-boundaries.md and ../11-engineering/testing-and-ci.md"
---

# Agent Contract: MUTE Runtime, Sandbox, and Diagnostics

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

- **Owns:** MUTE Runtime, Sandbox, and Diagnostics
- **Document:** `docs/05-mute/runtime-sandbox-and-errors.md`
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

- [ ] Implementation matches the requirements defined in `docs/05-mute/runtime-sandbox-and-errors.md`.
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

## Runtime implementation direction

- Lexer
- Parser
- AST or intermediate representation
- Semantic/type checks
- Interpreter
- Standard library
- Host API/capability layer
- Resource accounting
- Package validation integration
- Conformance tests
- CLI/test harness for developers

Use recursive-descent parsing for statements and a Pratt parser for expressions as an initial implementation strategy. Represent language control flow explicitly rather than confusing normal returns/breaks with runtime exceptions.

## Garbage collection

Because tables may contain cycles, a tracing garbage collector is a candidate. It should not be implemented until allocation patterns and runtime semantics are sufficiently defined, but reference counting alone can leak cycles.

## Sandbox must deny by default

No unrestricted:
- Filesystem access
- OS calls
- Sockets
- Native pointers
- Dynamic libraries
- Process creation
- Environment-variable or secret access
- Threads
- Reflection that bypasses capabilities

All host operations should be explicitly exposed and capability-checked.

## Resource budgets

Bound:
- Instruction count/time
- Memory and allocation
- Call stack depth
- Table/collection size
- Task count
- Callback count
- Network payload size
- Import depth/dependency count
- Package size and asset complexity

Budget exhaustion should fail predictably and may be uncatchable.

## Candidate error codes

- `MUTE_SYNTAX_ERROR`
- `NAME_ERROR`
- `TYPE_ERROR`
- `ARGUMENT_ERROR`
- `INDEX_ERROR`
- `IMPORT_ERROR`
- `PERMISSION_ERROR`
- `BUDGET_EXCEEDED`
- `BYTECODE_ERROR`
- `ASYNC_ERROR`
- `HOST_ERROR`

Errors should include source location and safe diagnostic context. Never leak secrets into logs.

## Tests and release gates

- Grammar/conformance tests
- Error message tests
- Budget enforcement tests
- Capability denial tests
- Fuzzing for parser and package boundaries
- Cross-platform parity tests
- Import/lockfile tests
- Regression suite for all previously fixed bugs
