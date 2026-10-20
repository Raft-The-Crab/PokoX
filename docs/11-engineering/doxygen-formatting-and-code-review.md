---
id: "docs-11-engineering-doxygen-formatting-and-code-review-md"
title: "Doxygen, Formatting, and Code Review"
kind: "implementation-contract"
execution_phase: "engineering-quality"
status: "specification"
source_of_truth: "This file plus ../02-architecture/module-boundaries.md and ../11-engineering/testing-and-ci.md"
---

# Agent Contract: Doxygen, Formatting, and Code Review

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

- **Owns:** Doxygen, Formatting, and Code Review
- **Document:** `docs/11-engineering/doxygen-formatting-and-code-review.md`
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

- [ ] Implementation matches the requirements defined in `docs/11-engineering/doxygen-formatting-and-code-review.md`.
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

## Doxygen policy

Public C++ APIs receive Doxygen comments in the same change that introduces them. Do not accumulate an undocumented public API backlog.

Document:
- Purpose and ownership
- Preconditions and valid ranges
- Return values and error behavior
- Resource lifetime/ownership
- Threading assumptions
- Platform restrictions
- Whether calls can fail and how failure is reported

Example:

```cpp
/**
 * @brief Describes the native surface used by the renderer.
 *
 * The native handle is interpreted only by the platform-specific renderer
 * integration. Callers retain ownership of the window.
 */
struct RenderSurface {
    void* nativeWindow = nullptr;
    std::uint32_t width = 0;
    std::uint32_t height = 0;
};
```

This is illustrative; the actual API may replace `void*` with a more explicit platform boundary.

## Doxyfile

Recommended starting settings:
- Project name: PokoX
- Input: `engine apps`
- Recursive source scanning
- HTML output enabled
- XML output enabled
- LaTeX output disabled initially
- Warnings enabled
- Warn for undocumented public APIs
- Private implementation details not extracted by default

Install Doxygen as a developer tool. It should not be a mandatory dependency for users of the built game.

Add an optional CMake documentation target using `find_package(Doxygen QUIET)` so a machine without Doxygen can still build the engine.

## Formatting and warnings

- Add `.clang-format`.
- Use consistent naming and include order.
- Enable compiler warnings.
- Resolve warnings rather than globally disabling them.
- Add `clang-tidy` later, after the build is stable and false positives can be managed.

## Review checklist

- Is the module owner correct?
- Does this introduce an unnecessary dependency?
- Are invalid inputs and errors handled?
- Is resource ownership clear?
- Are untrusted inputs bounded?
- Are public APIs documented?
- Are tests included?
- Are logs free of secrets?
- Is the behavior compatible with project/package versioning?
