# 33 — Implementation Backlog and Work Breakdown

This backlog converts the architecture into implementable work packages. Do not treat the list as a calendar promise. Prioritize one vertical slice and keep each task independently testable.

## M0 — Feasibility and repository

- [ ] Create monorepo and contribution/build instructions.
- [ ] Install MSYS2 UCRT64 toolchain, CMake, Ninja and SDL2.
- [ ] Build Diligent Core with selected compiler; record any required patches.
- [ ] SDL2 window + OpenGL context + Diligent triangle on Windows.
- [ ] Android NDK proof build + OpenGL ES surface.
- [ ] Test C ABI create/update/destroy and error handling.
- [ ] Set up CTest and CI for formatting/build/test.
- **Exit:** two supported platform proof builds or a documented blocker with an ADR.

## M1 — Engine playable slice

- [ ] Core lifecycle/logging/configuration.
- [ ] Math primitives and serialization-safe IDs.
- [ ] SDL input abstraction.
- [ ] Diligent device/resource/shader wrappers.
- [ ] Minimal render passes and camera.
- [ ] World, transforms, hierarchy and entity handles.
- [ ] AABB/plane/capsule collision and raycast.
- [ ] Character controller with fixed timestep.
- [ ] Asset import for initial glTF/GLB, PNG/JPEG, OGG/WAV.
- **Exit:** character walks in a packaged test world on Windows.

## M2 — MUTE v0.1

- [ ] Lexer, parser, AST and diagnostics.
- [ ] Define grammar and conformance tests.
- [ ] Interpreter and 1-based arrays.
- [ ] Dynamic runtime and optional static checking prototype.
- [ ] Client/Server/Shared/Studio realm registry.
- [ ] Capability API injection and permission-denial errors.
- [ ] Instruction/memory/call-depth/event budgets.
- [ ] Fuzz lexer/parser/bytecode decoder.
- **Exit:** client and server test scripts run in separate sandboxes; forbidden host access fails.

## M3 — Package container

- [ ] Define PPC binary header and version policy.
- [ ] Canonical manifest, file index and path rules.
- [ ] Chunk hashing and content-addressed store.
- [ ] SHA-256 verification and Ed25519 signing test vectors.
- [ ] Zstandard compression and decompression limits.
- [ ] `.pk`, `.pkx`, `.pkB` role separation.
- [ ] Safe package reader and fuzzing.
- **Exit:** build, sign, verify, corrupt and reject packages in tests.

## M4 — Bitstream and authority

- [ ] Choose transport using latency/reliability prototype.
- [ ] Define schema registry, envelope, sequence, lengths and version.
- [ ] Build headless C++ authority process.
- [ ] Player input, state snapshots and event replication.
- [ ] Movement/collision/interaction validation.
- [ ] Client prediction and reconciliation.
- [ ] Session tokens, replay protection and malformed packet tests.
- **Exit:** two clients play together; impossible actions are rejected.

## M5 — Creator Server SDK

- [ ] Server identity/key registration.
- [ ] Scoped short-lived credentials and revocation.
- [ ] Per-game tenancy and authorization tests.
- [ ] Official runtime packaging and Linux x86-64 build.
- [ ] DataStore, MemoryStore and Messaging adapter prototypes.
- [ ] Heartbeat, join tickets and server discovery.
- [ ] Per-server quotas and diagnostics.
- **Exit:** community server can register, host game, accept scoped join ticket and be revoked.

## M6 — Poko Studio MVP

- [ ] Project model and `.pk` persistence.
- [ ] World hierarchy/inspector/gizmos.
- [ ] UI document, responsive layout and preview.
- [ ] MUTE editor with diagnostics.
- [ ] Asset manager and import validation.
- [ ] Undo/redo command system and autosave.
- [ ] Local testing and package builder.
- **Exit:** creator builds and reopens a game project.

## M7 — Platform backend and player library

- [ ] Firebase identity mapping.
- [ ] Go modular monolith and migrations.
- [ ] PostgreSQL authoritative records.
- [ ] R2 private quarantine and approved content paths.
- [ ] Worker lightweight gateway and rate limits.
- [ ] Game catalog and release metadata.
- [ ] Download manager and local library.
- [ ] Offline modes and local save journal.
- **Exit:** player downloads an approved game and plays offline if declared supported.

## M8 — Publishing and Poko Mod

- [ ] Submission state machine and immutable hashes.
- [ ] Asset/game human review queue.
- [ ] Review Runtime as separate restricted process.
- [ ] Load exact `.pkx` and `.pkB` in review session.
- [ ] Scene/animation/script inspection tools.
- [ ] Evidence references, decision record and appeals.
- [ ] Moderator authentication/MFA and audit events.
- **Exit:** complete upload → quarantine → human review → signed publication process.

## M9 — Trust & Safety beta

- [ ] Publish policy and internal content labels.
- [ ] Quick Chat; text filter only after tests.
- [ ] Block/mute/report flows.
- [ ] Direct-to-platform reports.
- [ ] Safety incident escalation and evidence handling.
- [ ] Moderator staffing capacity gates.
- [ ] Privacy/retention and legal review per launch country.
- **Exit:** safety team can respond to reports and stop distribution.

## M10 — Poko Services and platform ecosystem

- [ ] DataStore async API and conditional updates.
- [ ] MemoryStore TTL semantics.
- [ ] Messaging and Teleport contracts.
- [ ] Profiles, discovery, parties and friends.
- [ ] Notifications and analytics.
- [ ] Server diagnostics and LiveOps constraints.
- **Exit:** creators can use documented game services without platform secrets.

## M11 — Marketplace and Pokoins

- [ ] Simulate weekly issuance and sinks.
- [ ] Pokoins double-entry ledger.
- [ ] Idempotent purchase transaction.
- [ ] 85/15 fee allocation.
- [ ] Refund/reversal policy.
- [ ] Asset/license ownership and dispute flow.
- [ ] Abuse/velocity limits and reconciliation reports.
- **Exit:** ledger invariants and rollback tests pass; moderation capacity supports marketplace.

## M12 — Android completion

- [ ] Kotlin shell and JNI lifecycle.
- [ ] OpenGL ES backend and device capability tiers.
- [ ] Touch/controller input.
- [ ] Storage/download and background lifecycle tests.
- [ ] UI Maker touch adaptation.
- [ ] Performance budgets on representative devices.
- **Exit:** Android PokoX plays, installs, updates and Studio opens shared projects.

## M13 — Public release readiness

- [ ] Restore drill for database and release objects.
- [ ] Security review and penetration test.
- [ ] Moderation capacity and legal readiness.
- [ ] Provider quota test and outage simulation.
- [ ] Incident response, support and status communications.
- [ ] Public policies and account/data lifecycle.
- **Exit:** all launch gates in `23_TESTING_QA_RELEASE.md` pass.

## Backlog discipline

Every task must have: owner, scope, dependencies, test plan, security impact, documentation impact and acceptance criterion. Do not mark tasks complete on UI appearance alone. Use small vertical slices and record blockers as ADRs.
