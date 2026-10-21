# PokoX — AI-Executable Engineering Plan

This repository is an **agent execution system** for building PokoX incrementally. It is not a promise that every listed subsystem exists, and it is not a request to build the whole platform at once.

## Start here (required order)

1. Read [`AGENTS.md`](AGENTS.md). These are the execution rules.
2. Read [`STATUS.md`](STATUS.md). Confirm actual repository state; do not trust stale status.
3. Read [`TASKS.md`](TASKS.md). Work on the first unblocked `READY` task only.
4. Read the task's linked contract under `docs/`.
5. Inspect the actual repository and create/update code in the same work session.
6. Run tests/build and record observed evidence.

## Product outcome

PokoX is a user-generated-content game platform with:
- **PokoX Player:** installs/opens and runs compatible game packages.
- **Poko Studio:** authors projects, scenes, gameplay, UI, assets, and packages.
- **PokoX runtime/engine:** executes projects and game packages.
- **MUTE:** a constrained scripting language/runtime.
- **Online services:** accounts, discovery, publishing, social, multiplayer, and eventually commerce.

## Non-negotiable sequencing

Build the Windows C++20/CMake foundation first. Prove a local offline vertical slice before online services. Android is a compatibility target after the Windows foundation is stable. Real-money commerce and open social/chat remain gated by security, moderation, legal, and operational readiness.

## Status semantics

- `READY`: can begin after confirming prerequisites in the actual repo.
- `BLOCKED`: cannot start until the listed dependency/evidence exists.
- `IN_PROGRESS`: a task is actively being implemented.
- `DONE`: implementation and acceptance tests passed; evidence is recorded.
- `DECISION_REQUIRED`: a real choice blocks implementation and needs a recorded ADR.

Do not mark design-only work as implementation complete.

## Repository map

- `AGENTS.md` — instructions for coding agents.
- `TASKS.md` — ordered executable task ledger.
- `STATUS.md` — current milestone, verified state, blockers.
- `docs/01-product/` — product contracts and MVP.
- `docs/02-architecture/` — module boundaries and lifecycle.
- `docs/03-engine/` — native engine.
- `docs/04-studio/` — authoring tools.
- `docs/05-mute/` — scripting language and runtime.
- `docs/06-content/` — project/package formats and publishing.
- `docs/07-online/` and `docs/08-marketplace/` — deferred services.
- `docs/09-trust-safety/` and `docs/12-security/` — required gates.
- `docs/11-engineering/` — testing, dependencies, and review.
- `docs/roadmap/` — staged delivery contracts.
- `docs/decisions/` — unresolved decisions and ADRs.

## Build commands

**Toolchain requirements:**
- CMake 3.24+
- C++20 compatible compiler (GCC 16.2.0 tested, Visual Studio 2022 preferred)
- Windows 11 x64

**Configure (Debug):**
```bash
cmake -B build/debug -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=C:/msys64/ucrt64/bin/gcc.exe -DCMAKE_CXX_COMPILER=C:/msys64/ucrt64/bin/g++.exe
```

**Build:**
```bash
cmake --build build/debug
```

**Run Tests:**
```bash
ctest --test-dir build/debug --output-on-failure
```


**Note:** Adjust compiler paths based on your toolchain. Visual Studio 2022 is the documented preferred toolchain; the above uses GCC as a fallback for environments without VS2022.

**Current Implementation:**
- PKX-001: Repository baseline with CMake, CTest, formatting, and Doxygen configuration
- PKX-002: CMake and automated test skeleton with CTest integration
- PKX-003: PokoCore library with Result/Error type, Logger with RAII, and EntityId (generational handles)

## Definition of done for any code task

A task is complete only when:
1. The implementation exists in the repository.
2. The relevant acceptance behavior is test-covered.
3. The supported build/test command was actually run successfully.
4. Failure cases and resource bounds are handled where relevant.
5. Public APIs and ownership are documented.
6. Task/status files include verification evidence.
