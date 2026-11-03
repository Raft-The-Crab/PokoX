# M0 — Repository Audit

**Date:** 2026-11-03
**Auditor:** primary engineering agent
**Method:** direct inspection of the local working tree, the GitHub REST API and
`git ls-remote`. Nothing below is inferred from filenames alone; where a claim is
a measurement, the command or API call that produced it is quoted.

---

## 1. Method and evidence

| Check | Command / endpoint | Result |
|---|---|---|
| Repository A metadata | `GET /repos/Raft-The-Crab/PokoX` | public, `default_branch=main`, `size=0`, `pushed_at=2026-11-03T02:13:01Z` |
| Repository A branches | `GET /repos/.../branches` | **HTTP 409** — GitHub returns this for a repository with no commits |
| Repository A commits | `GET /repos/.../commits` | **HTTP 409** — same reason |
| Repository A refs | `git ls-remote --heads https://github.com/Raft-The-Crab/PokoX.git` | empty output, exit 0 → no refs |
| Repository B metadata | `GET /repos/Raft-The-Crab/Mute` | public, `default_branch=main`, `size=0`, `pushed_at=2026-11-03T02:13:18Z` |
| Repository B branches/commits | `GET /repos/.../branches`, `/commits` | **HTTP 409** |
| Repository B refs | `git ls-remote --heads https://github.com/Raft-The-Crab/Mute.git` | empty output |
| Local working tree | `Test-Path .git` | **False** — the folder was not a clone |
| Local file inventory | recursive `Get-ChildItem` | 71 files, listed in §2 |
| Build health | `cmake -S . -B <temp> -G Ninja` | **exit 1**, see §3 |

`HTTP 409` on `branches` and empty `ls-remote` together mean the repositories
exist but contain **zero commits**. They are not private, and no credential was
required to read them, so access is not a blocker for this audit.

---

## 2. Repository A — what actually exists

71 files, all of them Markdown except one CMake file:

| Group | Count | Notes |
|---|---|---|
| `docs/00`–`docs/36` | 37 | the planning baseline |
| `adr/ADR-0001`–`0009` | 9 | architecture decisions |
| `README.md`, `CHANGELOG.md`, `REFERENCES.md`, `MANIFEST.md` | 4 | |
| directory `README.md` stubs | 21 | one per top-level directory |
| `CMakeLists.txt` | 1 | non-functional, see §3 |
| `.gitignore` | 1 | contains two harmful rules, see §4 |

**There is no source code, no build script other than the broken root
`CMakeLists.txt`, no CI workflow and no test.**

`MANIFEST.md` advertises 37 documents and 9 ADRs; both counts are correct, and
the listed SHA-256 hashes were not modified by this work.

### 2.1 Directory scaffolding

The tree contained **1,468 directories**, of which **1,461 were empty leaves**
such as `mute/vm/stack/growth/`, `backend/api/v2/` and
`cmake/modules/diligent/opengl/`. Git cannot store an empty directory, so this
structure was never going to reach the repository — it was a diagram rendered as
folders, and it implied design decisions (a `vm/` split, an API `v2/`) that no
code or ADR had validated.

Those 1,461 directories were removed. All 21 README stubs and all 37 docs were
kept.

### 2.2 MUTE separation

The tree contained `mute/` at top level with a full language-shaped layout —
`lexer/`, `parser/`, `ast/`, `interpreter/`, `vm/`, `libraries/`, `tools/`,
`runtime/` — plus a `mute/README.md` describing the language implementation.

This conflicts with the mandatory separation rule. `engine/README.md` already
declares the correct PokoX-side module (`engine/mute`: "MUTE runtime integration
and host capability bridge"), so the top-level tree was a second, competing
home for the language.

The top-level `mute/` tree was removed. Its content lives on in
[`05-mute-integration.md`](./05-mute-integration.md) and the PokoX-side binding
module stays at `engine/mute/`.

---

## 3. Build state — broken by construction

The single CMake file made 13 `add_subdirectory()` calls into directories with
no `CMakeLists.txt`. A configure was run against a throwaway build directory:

```
-- The CXX compiler identification is GNU 16.2.0
CMake Error at CMakeLists.txt:29 (add_subdirectory):
  The source directory C:/.../PokoX/engine/core does not contain a CMakeLists.txt file.
CMake Error at CMakeLists.txt:30 (add_subdirectory):
  The source directory C:/.../PokoX/engine/math does not contain a CMakeLists.txt file.
...
EXITCODE=1
```

So the repository could not configure at all, and no claim in the planning
documents about "build setup" could have been backed by a running build.

The replacement keeps the declared intent (every module named in
`docs/03_ENGINE_ARCHITECTURE.md` §8) but adds each module **only when it
exists**, reporting every skip at configure time. The build now always reflects
what is really in the tree.

---

## 4. Defects found in existing configuration

| # | Defect | Risk | Resolution |
|---|---|---|---|
| 1 | `.gitignore` ignored `*.cmake` globally, re-including only `cmake/toolchains/*.cmake` and a non-recursive `cmake/modules/*.cmake` | Real CMake modules (dependency pins, helpers) would be silently untracked — a reproducibility failure that only surfaces on a clean clone | blanket rule removed |
| 2 | `.gitignore` ignored `*.a`, `*.lib`, `lib/`, `bin/` unanchored | any vendored static lib or `bin/` under a source tree is excluded without warning | replaced with anchored build-output rules |
| 3 | No `.gitattributes`; Git warned about LF→CRLF on every file | CRLF in CMake/shell fragments breaks MSYS2 builds and makes patches noisy | added, `text=auto eol=lf` |
| 4 | `docs/26_BUILD_ENVIRONMENT_AND_CMAKE.md` §3 describes `documentation/`, the tree uses `docs/` | doc drift | recorded, doc not rewritten (it is a planning document) |
| 5 | No `LICENSE` at repository root | cannot publish binaries, and dependency licence obligations have no anchor | open gate, see §6 |

---

## 5. Repository B — Mute

The repository exists, is public, and contains **zero commits**. It was not
cloned because there is nothing to clone.

Consequences for M0, stated plainly:

- There is no lexer, parser, interpreter, CMake target or embedding header to
  inspect.
- "Mute compiles and tests independently" is **not verifiable** and is not
  claimed.
- The M0-F deliverable can therefore only be a *proposal*: an exported CMake
  target, a C++ embedding API sketch, a version-pinning strategy and the
  submodule-versus-FetchContent evaluation. See
  [`05-mute-integration.md`](./05-mute-integration.md).
- Nothing in Mute can be overwritten, because nothing exists.

---

## 6. Open gates carried out of this audit

1. **No licence.** Cannot ship binaries or accept contributions until chosen.
2. **Mute is empty.** M2 (language v0.1) cannot start; M0-F stays a proposal.
3. **Push authentication is not configured.** Git identity was
   `John <john.dev@example.com>` with no credential helper and no `GITHUB_TOKEN`.
   It was corrected locally to the project owner; pushing still requires
   interactive authentication or a token, and no push was performed.
4. **No CI.** `.github/workflows/{ci,cd,docs,security}` are empty directories.
   GitHub Actions cannot be validated until there is something to build on a
   clean machine — recommended as the first M1 task.
5. **Planning documents are marked `Proposed`.** `docs/26` §5 says the first
   milestone should prove SDL2 + Diligent context ownership; that is exactly what
   M0-C did, and its result is recorded in
   [`02-toolchain-report.md`](./02-toolchain-report.md).
