# ADR-0013 — MUTE is consumed as a pinned external dependency, never vendored

**Status:** Accepted
**Date:** 2026-11-03
**Scope:** PokoX repository; the language itself is specified in the Mute repository

## Context

MUTE is an independent programming language with its own lexer, parser, AST,
compiler, type checker, interpreter, future bytecode VM, sandbox and standard
library. It is developed in `Raft-The-Crab/Mute`.

The planning package created a top-level `mute/` tree inside PokoX with a full
language-shaped layout (`lexer/`, `parser/`, `ast/`, `interpreter/`, `vm/`,
`libraries/`, `tools/`, `runtime/`) and a README describing the language
implementation. It was empty, but it established a second home for the language,
and the natural next step — copying Mute sources in "temporarily" — would have
been one commit away.

At the time of this decision the Mute repository contains **zero commits**, so
there is nothing to vendor even if that were desirable.

## Decision

1. **The `mute/` tree was removed from PokoX.** `engine/README.md` already
   declares the correct PokoX-side module — `engine/mute`: *"MUTE runtime
   integration and host capability bridge"*. That module holds adapters,
   capability grants, realm wiring and budget policy. It will never contain a
   lexer, parser or VM.

2. **Dependency direction is one-way.** PokoX depends on Mute; Mute knows nothing
   about PokoX. Mute must build and test with no PokoX sources present.

3. **Acquisition is `FetchContent` at a pinned tag, verified against a reviewed
   commit SHA**, exposed to consumers as an exported CMake target `Mute::Mute`.
   A `find_package(Mute CONFIG)` path must also work, so switching acquisition
   strategies later does not touch consumer code.

4. **Submodules are rejected for this dependency.** A submodule makes editing the
   language while working on the engine almost effortless — which is precisely
   the failure the separation rule exists to prevent. `FetchContent` content is
   read-only from PokoX's point of view.
   Accepted cost: a clean build needs one network fetch, recorded in
   `<build>/poko-dependency-pins.txt`.

5. **The embed boundary is a C ABI with opaque handles** — the same rules as
   ADR-0012: versioned structs, integer status codes, no C++ types or exceptions
   across the boundary, host capabilities passed as explicit function-pointer
   tables rather than ambient authority.

6. **No placeholder integration target exists yet.** An empty `poko_mute` target
   would tick a box while proving nothing, and would encourage code written
   against an interface that does not exist. Integration starts when Mute has its
   first commit.

## Consequences

- MUTE work cannot accidentally happen in the PokoX repository, because there is
  no place for it to happen.
- PokoX cannot start M2 until Mute exists. That is a scheduling constraint, not a
  technical blocker, and it is recorded in the M0 report.
- The contract Mute must satisfy (CMake target, embedding API shape, version
  compatibility, test obligations) is specified in
  `docs/engineering/m0/05-mute-integration.md` so PokoX is not blocked on
  guesswork when Mute's first commit lands.

## Alternatives considered

- **Copy the language into `engine/mute`.** Rejected: violates the mandatory
  repository separation, doubles maintenance, and makes it impossible to test the
  language independently.
- **Git submodule at `third_party/Mute`.** Rejected for the edit-in-place reason
  above.
- **Treat MUTE as a separate process with an IPC protocol from day one.**
  Rejected for M0/M2: it would add serialisation cost and a protocol to design
  before the language's own semantics are stable. Sandboxing is achieved with
  capabilities and budgets inside the process first; a process boundary can be
  added later for the Review Runtime, where it is genuinely required.
