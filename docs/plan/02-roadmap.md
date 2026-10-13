# Roadmap and Exit Gates

## Phase 0 — Audit
- [ ] Record Poko/Mute commits and working-tree status.
- [ ] Build clean from documented instructions.
- [ ] Run tests and capture failures.
- [ ] Inventory dependencies, licenses, dead code, duplicate state, and incomplete implementations.
- [ ] Mark every subsystem Verified/Partial/Absent/Broken.
- [ ] Add CI, format, lint, and test jobs.

**Gate:** a clean clone can build and reproduce test status.

## Phase 1 — Language boundary experiment
- [ ] Build a minimal Rust library with a C ABI.
- [ ] Build a minimal EGUI + eframe application for Windows.
- [ ] Build a minimal Kotlin + JNI bridge to Rust engine for Android.
- [ ] Define allocation, error, handle, and shutdown rules.
- [ ] Test invalid inputs and repeated calls.
- [ ] Prove Android cross-compilation before promising parity.
- [ ] Write a go/no-go decision for Rust migration.

**Gate:** the native boundary has automated tests and documented ownership.

## Phase 2 — Engine vertical slice
- [ ] Object IDs/handles, lifecycle, transforms, metadata.
- [ ] Minimal Diligent render path and sample scene.
- [ ] Asset load/unload, audio playback, logging and profiling.
- [ ] Physics test harness and basic collision/query tests.

**Gate:** a small scene runs reproducibly with measurements.

## Phase 3 — Mute
- [ ] Freeze syntax/grammar and diagnostics.
- [ ] Lexer, parser, semantic checks, capabilities.
- [ ] Versioned bytecode and verifier.
- [ ] VM instruction/memory/recursion limits.
- [ ] Engine binding and script tests.
- [ ] Local language server/autocomplete.

**Gate:** scripts run safely and malformed bytecode is rejected.

## Phase 4 — Studio creator loop
- [ ] Create/open project, hierarchy, viewport, inspector.
- [ ] Mute editor diagnostics and output panel.
- [ ] Game UI editor, save/reopen, undo/redo.
- [ ] Local playtest, asset import, package validation.
- [ ] Publish immutable version and rollback pointer.

**Gate:** creator can build, test, save, reopen, and publish a small experience.

## Phase 5 — Accounts and multiplayer
- [ ] Auth/session/revocation.
- [ ] Join/leave and C++ authority protocol.
- [ ] Basic replication, reconnect and invalid-request handling.
- [ ] Reports, mute/block, moderation audit.
- [ ] Load tests with realistic traffic.

**Gate:** two clients can play a test experience; invalid requests cannot corrupt platform data.

## Phase 6 — Public platform
- [ ] Discovery, profiles, favorites/history.
- [ ] Publishing permissions, moderation queues, enforcement.
- [ ] Metrics, alerts, backup/restore.
- [ ] Economy only after fraud controls and payment/legal review.

## Phase 7 — Expansion
- [ ] Marketplace, plugins, achievements.
- [ ] GitHub integration and optional developer-hosted/VPS workflows.
- [ ] Advanced physics/rendering based on benchmarks.
- [ ] More regions/device tiers after measurement.

Never advance because code merely exists. Advance when exit criteria pass.
