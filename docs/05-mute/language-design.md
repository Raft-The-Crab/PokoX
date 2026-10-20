---
id: "docs-05-mute-language-design-md"
title: "MUTE Language Design"
kind: "implementation-contract"
execution_phase: "later-feature"
status: "specification"
source_of_truth: "This file plus ../02-architecture/module-boundaries.md and ../11-engineering/testing-and-ci.md"
---

# Agent Contract: MUTE Language Design

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

- **Owns:** MUTE Language Design
- **Document:** `docs/05-mute/language-design.md`
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

- [ ] Implementation matches the requirements defined in `docs/05-mute/language-design.md`.
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

## Purpose

MUTE is PokoX's own sandboxed gameplay and package scripting language. It should be approachable, predictable, and safe to host inside games.

The language design below is a proposal, not a finished compiler specification. Grammar and APIs must be made internally consistent and tested with a conformance suite before the language is declared stable.

## Initial direction

- Lua-inspired block syntax using `end`
- Dynamic runtime with optional static checking
- One-based arrays
- Interpreter first, bytecode VM later if profiling justifies it
- No native libraries in the first release
- Schema-defined Bitstream networking
- Host APIs separated by capability and client/server context

## Source

- File extension: `.mute`
- UTF-8 source
- ASCII identifiers initially
- Case-sensitive identifiers
- Line comments: `--`
- Block comments: `--[[ ... ]]`

## Candidate keywords

`and`, `break`, `do`, `else`, `elseif`, `end`, `false`, `for`, `function`, `if`, `in`, `local`, `nil`, `not`, `or`, `repeat`, `return`, `then`, `true`, `until`, `while`, `continue`, `type`, `import`, `as`, `export`, `await`, `try`, `catch`, `throw`.

This list must be reconciled with the actual grammar before implementation. Do not implement keywords without defined syntax and semantics.

## Runtime values

Candidate values:
- `nil`
- boolean
- IEEE-754 binary64 number
- UTF-8 string
- function
- table
- object handle
- managed task

Truthiness: only `false` and `nil` are falsey. Public host APIs should reject non-finite numbers where those values would be unsafe.

## Tables and collections

- Tables can act as arrays or maps.
- Array indices start at 1.
- `nil` map key is invalid.
- Assigning `nil` removes a key.
- `#` is defined for contiguous sequences; empty sequence length is 0.
- Sparse arrays need explicit length/iteration rules.
- Map iteration order is unspecified.
- In-memory cycles may be allowed, but default network serialization must reject cycles unless a schema explicitly defines a safe representation.

## Control flow

- Numeric loop start/end/step are evaluated once.
- Default numeric step is 1.
- Zero step is an error.
- `break` and `continue` only apply inside loops.
- `repeat` body runs at least once.
- `try/catch` handles ordinary runtime errors; budget/security errors may be uncatchable.

## Important specification gap

Earlier examples used compound assignment such as `+=`, but the candidate keyword list/grammar does not yet define it. Function expressions, imports, `await`, `try/catch`, `throw`, and exact type syntax also need a consistent formal grammar. Resolve these before treating examples as normative.
