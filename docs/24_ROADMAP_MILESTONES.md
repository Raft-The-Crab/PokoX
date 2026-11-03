# 24 — Development Roadmap and Milestone Gates

**Status:** Proposed order; dates intentionally not estimated. A $0 solo project should plan by validated milestones, not optimistic calendar promises.

## 1. Guiding sequence

Build the riskiest technical vertical slice first. Avoid developing all three application UIs in parallel before shared engine and package assumptions are proven.

## 2. Milestones

### M0 — Repository and feasibility

- MSYS2 UCRT64 setup.
- CMake/Ninja toolchain.
- Pin Diligent Core and SDL2 versions.
- Build Diligent with selected compiler.
- Create SDL window and Diligent OpenGL device.
- Draw triangle; resize; clean shutdown.
- Create Android NDK/SDL/Diligent GLES sample separately.
- Record build blockers and licenses.

**Exit:** reproducible Windows build and verified Android feasibility path.

### M1 — Engine vertical slice

- Core/math/platform/input.
- Minimal renderer and world.
- Character capsule and basic collision.
- Asset load.
- Simple UI.
- C ABI host.

**Exit:** character moves around a test scene on Windows; Android architecture proof exists.

### M2 — MUTE interpreter v0.1

- Lexer/parser/AST/interpreter.
- Language semantics.
- Client/server/shared contexts.
- Capability checks and budgets.
- Tests/fuzzer.

**Exit:** untrusted scripts cannot access host OS or cross realms and runaway scripts are contained.

### M3 — PPC and file formats

- Common container and manifest.
- `.pk`, `.pkx`, `.pkB` roles.
- Hashes/signatures.
- Chunk cache.
- Safe package reader.

**Exit:** build, sign, verify, install and load a test package; tampering is detected.

### M4 — Multiplayer authority prototype

- Bitstream schema registry.
- Transport ADR/prototype.
- Session handshake.
- Authoritative player state.
- Client prediction/reconciliation.
- Simple replication.

**Exit:** two players join and impossible movement/action is rejected.

### M5 — Game server SDK

- Server registration and per-instance keys.
- Scoped Poko Services.
- Heartbeats and revocation.
- DataStore namespace prototype.
- Creator-hosted Linux runtime proof.

**Exit:** creator server can host approved test game without access to PokoX platform secrets.

### M6 — Poko Studio MVP

- Project manager.
- World Editor baseline.
- UI Maker baseline.
- MUTE Editor.
- Asset Manager.
- Test Lab.
- `.pk` save and build output.

**Exit:** external tester can create a simple playable game without editing engine source.

### M7 — Platform backend and PokoX library

- Firebase identity mapping.
- Go modular monolith.
- PostgreSQL migrations.
- R2 package storage.
- Game catalog.
- Download/install/update.
- Local offline mode and saves.

**Exit:** a player downloads and launches an approved offline-ready game.

### M8 — Publishing and Poko Mod

- Quarantine.
- Technical scan.
- Human review queue.
- Shared isolated Review Runtime.
- Evidence/decision/appeal records.
- Hash-bound release approval.

**Exit:** exact submitted `.pkx`/`.pkB` can be reviewed and published; changed package is rejected until re-reviewed.

### M9 — Trust & Safety beta

- Content policy and labels.
- Reports/block/mute/quick chat.
- Privacy defaults.
- Moderator procedures/training.
- Sensitive incident escalation.

**Exit:** safety staffing and workflows can handle actual beta volume. If not, keep beta invite-only.

### M10 — Social and creator marketplace foundations

- Profiles/favorites/friends.
- Asset/library marketplace listing.
- License/ownership records.
- Creator analytics.

**Exit:** marketplace items have clear approval, version and license lifecycle.

### M11 — Pokoins activation

- Ledger.
- Weekly task issuance.
- 85/15 fee.
- Economy monitoring.
- Refunds/reversals.
- Abuse detection.

**Exit:** financial-style ledger tests and audit checks pass. Keep Pokoins disabled until this gate.

### M12 — Android client and Studio completion

- Kotlin app integration.
- Touch input and responsive UI.
- GLES validation.
- Lifecycle handling.
- Download/storage behavior.
- Device compatibility tests.

**Exit:** supported Android devices can play and create within documented feature limits.

### M13 — Public release preparation

- External security review.
- Jurisdiction review.
- Operational funding/capacity plan.
- Backups/restore.
- Incident response.
- Moderator capacity.
- Support and documentation.

**Exit:** public service limits, safety obligations, support path and recovery capability are documented and staffed.

## 3. Solo developer reality

A C++ engine, programming language, game platform, backend, three applications, custom physics and moderation operation is a very large multi-year effort for one developer. Reduce scope aggressively. First produce a small game with a verified package and two-player server session. Do not implement avatar marketplace, social recommendations, Android Studio, creator payouts and advanced physics before that proof.

## 4. Weekly work discipline

For each milestone: write a design note → prototype the riskiest assumption → define tests → implement smallest slice → run tests → document result → update decision register. Avoid starting another subsystem when the current one has no passing acceptance test.

## 5. Scope control

Every new feature must state: user problem, owning module, security boundary, data model, failure behavior, API contract, test criteria, platform cost, and whether it blocks MVP. Features lacking these should remain ideas, not implementation commitments.

## 6. Final release sequencing constraints

- M0 must prove MSYS2 UCRT64 + SDL2 + Diligent Core OpenGL context ownership and Android NDK/OpenGL ES viability before broad engine work.
- MUTE sandbox tests must pass before creator MUTE runs against public players.
- PPC parser fuzzing and signature verification must pass before opening package uploads.
- Poko Mod review process isolation must pass before human reviewers inspect untrusted public packages.
- Creator Server SDK cross-tenant authorization tests must pass before public server registration.
- Manual moderation capacity must be staffed and measured before public submissions are accepted.
- DataStore restore, economy ledger invariants and idempotency tests must pass before Pokoins or paid-like virtual purchases are activated.
- Android support is preserved architecturally from M0, but the complete Android Studio/client experience is a later milestone.

## 7. Final milestone exit rule

A milestone is not complete because code compiles or a UI screen exists. It is complete only when the documented acceptance tests pass, a reproducible build exists, relevant security tests pass, documentation is updated and the owner signs off the milestone record. Dates are intentionally omitted: the schedule depends on solo-developer capacity and prototype results.
