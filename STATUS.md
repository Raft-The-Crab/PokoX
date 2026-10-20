# STATUS.md — Verified Project State

## Current milestone

**Milestone:** Phase 0 — repository/build baseline inspection  
**Current task:** `PKX-001`  
**Project implementation status:** Not yet verified by this plan-conversion step.

## Important truthfulness rule

This document is not evidence that code exists or that any build passes. The coding agent must inspect the current repository and replace the state below with observed facts.

## Verified environment

- [ ] Repository tree inspected
- [ ] Compiler/toolchain version recorded
- [ ] CMake version recorded
- [ ] Clean configure/build attempted
- [ ] Tests executed
- [ ] CI status inspected

## Milestone state

| Task | Status | Evidence |
|---|---|---|
| PKX-001 Repository baseline | READY | Not inspected in this conversion |
| PKX-002 CMake/test skeleton | BLOCKED | Requires PKX-001 |
| PKX-003 Core primitives | BLOCKED | Requires PKX-002 |
| PKX-004 Windows lifecycle | BLOCKED | Requires PKX-002 and PKX-003 |
| PKX-010+ Engine and product slices | BLOCKED | Dependency chain in `TASKS.md` |

## Latest verified change

- Reworked the plan into an AI-executable contract structure.
- No source code build/test was performed by the plan conversion.
- Next action: execute `PKX-001` against the actual source repository.

## Update format

For each update, record:
- Date/time
- Task ID
- Files changed
- Exact commands executed
- Pass/fail result
- Remaining blocker
- Next task ID
