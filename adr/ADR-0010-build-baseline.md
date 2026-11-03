# ADR-0010 — Build baseline: CMake modules, pinned dependencies, honest targets

**Status:** Accepted
**Date:** 2026-11-03
**Supersedes:** the placeholder root `CMakeLists.txt` from the planning package

## Context

The planning package shipped a root `CMakeLists.txt` with 13
`add_subdirectory()` calls into directories that contained no `CMakeLists.txt`.
Configure failed at line 29 with exit code 1, so no statement in `docs/26`
about the build could ever have been verified.

Two further problems were found in the same area:

- `.gitignore` ignored `*.cmake` globally. Any real CMake module added later
  (dependency pins, toolchain files, helpers) would have been silently excluded
  from version control — a reproducibility failure that only appears on a clean
  clone, i.e. exactly when it is most expensive to discover.
- The tree contained 1,461 empty leaf directories, implying module splits and an
  API `v2` that no code or ADR justified. Git cannot store empty directories, so
  none of it was ever going to reach the repository.

## Decision

1. **Modules are added only when they exist.**
   `poko_add_component_if_present(<path>)` adds a directory only if it contains
   a `CMakeLists.txt` and prints a one-line status message otherwise. The
   declared module set stays exactly as named in
   `docs/03_ENGINE_ARCHITECTURE.md` §8, so the intent is preserved, but the build
   never claims to contain something it does not.

2. **Third-party code is pinned by content or by commit, never by branch.**
   - SDL2: consumed from the MSYS2 UCRT64 package (`2.32.10`). Exactly one SDL2
     exists in the process, so the "system SDL2 vs compiled SDL2" hazard in
     `docs/26` §3 cannot occur on Windows. Android will build SDL2 from a pinned
     source revision under its own toolchain.
   - Diligent Core: acquired with `FetchContent` from a release archive pinned by
     **SHA-256 content hash** (`SHA256=` + `URL_HASH`), so CMake fails the
     configure if a single byte differs. A content hash is a stronger pin than a
     tag: a re-tagged or regenerated upstream cannot silently change what is
     built. The upstream commit the tag pointed to at review time is recorded
     alongside the hash for traceability, and every pin is written to
     `<build>/poko-dependency-pins.txt`.
   - Reason for the archive instead of `GIT_REPOSITORY`: `git clone --depth 1` of
     the same repository stalled on the development network (pack file received
     zero bytes, reproduced twice), while the archive transfers normally. This is
     recorded because it is an environmental fact that a future contributor will
     otherwise rediscover.

3. **Include paths and warnings are target-scoped.**
   `poko_set_warnings()` applies per target; there is no global `-Werror` and no
   global `include_directories()`. A third-party or generated target can opt out
   without weakening the rest of the build.

4. **`BUILD_TESTING` from `include(CTest)` replaces the custom
   `POKO_BUILD_TESTS`**, so `ctest` behaves the way every other CMake project
   expects.

5. **Executables land in one directory** (`<build>/bin`) and Windows executables
   get their runtime DLLs copied next to them, so a binary launched outside the
   MSYS2 shell behaves the same as one launched inside it.

## Consequences

- A configure on this tree now succeeds and reports what is missing instead of
  failing.
- Adding `engine/math/CMakeLists.txt` in M1 requires no edit to the root file.
- A clean build needs network access once per pinned dependency; the pin file
  makes an existing build auditable after the fact.
- The removed empty directories must be recreated with real content. This is
  intentional: a directory should exist because something lives in it.

## Alternatives considered

- **Keep the scaffolding and generate stub `CMakeLists.txt` files** so configure
  passes. Rejected: 13 empty libraries that build and link nothing would hide
  real gaps and invite code written against targets with no behaviour.
- **Git submodules for both dependencies.** Rejected for M0: a submodule
  requires an extra `git submodule update --init` step that is easy to forget and
  easy to leave dirty, and it encourages editing third-party code in place. See
  ADR-0013 for the fuller comparison, which applies to language runtimes and
  graphics libraries alike.
