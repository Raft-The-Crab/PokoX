# M0-F — MUTE Integration Assessment and Proposal

**Status:** proposal only. **No integration code was written**, because there is
nothing to integrate yet: `Raft-The-Crab/Mute` exists but contains **zero
commits** (verified by `GET /repos/.../branches` → HTTP 409 and an empty
`git ls-remote --heads`). See [`01-repository-audit.md`](./01-repository-audit.md).

This document therefore defines the *contract* Mute must satisfy before PokoX
can consume it, so that the dependency can be pinned the moment Mute has code —
without PokoX inventing language decisions that belong to the Mute repository.

---

## 1. Non-negotiable constraint

Both repositories stay independent. PokoX consumes Mute as an **external,
versioned dependency**; Mute never learns about PokoX.

```
      PokoX (C++ engine, apps, backend)          Mute (lexer → VM)
                    │                                   │
                    │  depends on, pinned revision      │  knows nothing
                    └──────────────────────────────────►│  about PokoX
```

Concretely, inside PokoX:

| Belongs to Mute | Belongs to PokoX (`engine/mute/`) |
|---|---|
| lexer, parser, AST, type checker | host-binding adapters (`Players`, `World`, `UI`, `DataStore`, `Network`) |
| interpreter, values, GC | realm policy enforcement (client/server/shared/studio) |
| bytecode compiler and VM | capability grants derived from package permissions |
| standard library (`math`, `string`, `table`, `time`) | resource budgets tied to platform limits |
| language tests and language docs | script *content* validation and moderation hooks |

No MUTE source file may ever be copied into PokoX. A `mute/` tree at the top of
PokoX was removed during this milestone precisely because it re-created a second
home for the language.

---

## 2. Evaluation: FetchContent versus submodule

| Criterion | `FetchContent` (pinned tag + reviewed SHA) | Git submodule |
|---|---|---|
| Clone experience | plain `git clone` + one CMake configure | requires `git submodule update --init --recursive` |
| Pin visibility | one line in `cmake/modules/PokoMute.cmake` | `.gitmodules` + submodule pointer, easy to miss |
| Offline builds | needs one network fetch, then cached under `_deps` | works offline after init |
| Contributing to Mute from PokoX | clone Mute separately | built in, tempting to edit in place |
| Accidental modification | impossible (read-only content) | possible, and easy to commit by mistake |
| Reproducibility | tag + verified SHA recorded in the build tree | commit pointer is exact by construction |

**Decision for M0/M1: `FetchContent`, pinned to a tag, verified against a
reviewed commit SHA.** The deciding factor is not convenience — it is the rule
that MUTE implementation work must not happen inside PokoX. A submodule makes
in-place editing of the language while working on the engine almost effortless,
which is exactly the failure mode the separation rule exists to prevent.

The trade-off accepted: a clean build needs network access once. That is already
true of Diligent Core, and the pin is recorded in
`build/<preset>/poko-dependency-pins.txt` so a build can be audited afterwards.

Recorded SHA verification is not decorative: `PokoDependencies.cmake` compares
the resolved commit against the reviewed pin and warns loudly on a mismatch,
because tags can be moved upstream.

---

## 3. Contract Mute must satisfy

These are requirements on the **Mute** repository, to be implemented there, not
in PokoX. They are stated here so PokoX integration is not blocked on guesswork.

### 3.1 Build

| Requirement | Rationale |
|---|---|
| CMake ≥ 3.24, Ninja-friendly, no generator lock-in | matches PokoX |
| C++20, no compiler extensions | matches PokoX |
| Builds and tests with **zero PokoX code present** | proves independence |
| Exports `Mute::Mute` via `install(EXPORT)` + `MuteConfig.cmake` | lets PokoX switch to `find_package` without touching consumer code |
| Also usable as `add_subdirectory` (no global flags, no `CMAKE_SOURCE_DIR` assumptions) | FetchContent path |
| No dependency on SDL2, Diligent, or any platform library | a language runtime must run headless on a creator's server |
| Optional `MUTE_NO_EXCEPTIONS` build | server and review-realm binaries should not unwind |

### 3.2 Embedding API — shape, not implementation

An opaque-handle C ABI, for the same reason the engine ABI is one: PokoX must be
able to embed Mute from C++, Rust (host tooling) and, later, a JNI bridge
without recompiling against Mute's internal C++ types.

```c
/* Proposed public surface; lives in the Mute repository. */
#define MUTE_EMBEDDING_API_VERSION 1u

typedef struct mute_vm       mute_vm;
typedef struct mute_program  mute_program;

typedef enum mute_realm {
    MUTE_REALM_CLIENT  = 0,
    MUTE_REALM_SERVER  = 1,
    MUTE_REALM_SHARED  = 2,
    MUTE_REALM_STUDIO  = 3
} mute_realm;

typedef struct mute_budget {
    uint32_t struct_size;
    uint32_t api_version;
    uint64_t instruction_limit;
    uint64_t memory_limit_bytes;
    uint32_t call_depth_limit;
} mute_budget;

/* Host capabilities are a table of function pointers, never raw engine
   pointers: Mute may only call what the table exposes. */
typedef struct mute_host_api {
    uint32_t struct_size;
    uint32_t api_version;
    void*    user_data;
    /* ... one vtable per capability namespace (world, ui, datastore, network) */
} mute_host_api;

uint32_t mute_embedding_api_version(void);

int32_t  mute_vm_create(const mute_host_api* host,
                        mute_realm realm,
                        const mute_budget* budget,
                        mute_vm** out_vm);

int32_t  mute_compile(mute_vm* vm,
                      const char* chunk_name,
                      const char* source_utf8,
                      size_t source_size,
                      mute_program** out_program);

int32_t  mute_invoke(mute_vm* vm, mute_program* program,
                     const char* entry_point, uint32_t argc, /* ... */);

void     mute_program_destroy(mute_program* program);
void     mute_vm_destroy(mute_vm* vm);
```

Rules that mirror the engine ABI decisions already proven in M0:

- opaque handles, integer status codes, `struct_size` + `api_version` on every
  struct that can grow;
- no exceptions, STL containers, or C++ types across the boundary;
- one `mute_vm` per realm per script sandbox — no shared mutable global state,
  because a client realm and a server realm must never observe each other;
- host capabilities are explicit function-pointer tables; there is no ambient
  authority to reach the filesystem, processes, sockets or native code.

### 3.3 Version compatibility

| Layer | Mechanism |
|---|---|
| Source pin | tag in `PokoMute.cmake` + reviewed SHA verified at configure time |
| CMake package | `MuteConfigVersion.cmake` with `SameMajorVersion` compatibility |
| Runtime | `mute_embedding_api_version()` compared against the value PokoX was compiled with, at VM creation, returning a distinct error |
| Language | `LANGUAGE_VERSION` constant; a `.pk`/`.pkx` manifest declares the language version it needs |

PokoX must never silently accept a Mute whose embedding ABI differs. A game
package that declares an unsupported language version must be rejected at
publish time, by the same code path the client uses — not only at load time.

---

## 4. Where the integration code will live

| Path (PokoX) | Purpose | Milestone |
|---|---|---|
| `engine/mute/` | `poko_mute` target: capabilities, budget policy, realm wiring, host bindings | M1/M2 |
| `engine/mute/bindings/` | `Players`, `World`, `UI`, `Network`, `DataStore` adapters — PokoX APIs, not language APIs | M2 |
| `cmake/modules/PokoMute.cmake` | pin + FetchContent + SHA verification | when Mute has its first commit |

**No placeholder target or module was created in M0.** An empty `poko_mute`
target would satisfy a checklist while proving nothing, and would invite code to
be written against an interface that does not exist yet.

---

## 5. What PokoX must never delegate to a creator script

Restating the trust boundary that this integration has to preserve, because it is
the reason the sandbox exists at all:

- Pokoins issuance, balances and the transaction ledger;
- platform authentication, session tokens and account state;
- other games' data, other players' private data;
- global moderation decisions and account enforcement;
- package signing keys and platform administration;
- offline reward verification.

Client-side MUTE execution is presentation and prediction only. Any client
script result that affects authoritative state must be re-derived or validated
by a server, whether that server is PokoX-hosted or community-hosted.

---

## 6. Test obligations before MUTE runs against real players

To be implemented in the Mute repository, and re-run from PokoX as an
integration suite:

1. Realm isolation: a server-realm script cannot see client-realm globals.
2. Capability denial: filesystem, process, socket and native-load attempts fail
   with a defined error, not a crash.
3. Budgets: instruction, memory, call-depth and subscription limits terminate a
   hostile script deterministically and leave the host process healthy.
4. Parser fuzzing: the lexer/parser survive malformed input (libFuzzer or AFL++).
5. Determinism: shared-realm utilities produce identical output across client
   and server for the same input, or are excluded from shared realm.
6. Embedding ABI: the version check rejects a mismatched runtime, and a denied
   capability cannot be reached through a host-table hole.

---

## 7. Decisions this document does *not* make

Language design belongs to the Mute repository. PokoX deliberately takes no
position here on: syntax details beyond the recorded Lua-style blocks and
one-based indexing, optional static typing rules, garbage-collection strategy,
bytecode format, module system, or standard-library surface. Those are recorded
in the Mute repository's own documentation, and PokoX pins the result.
