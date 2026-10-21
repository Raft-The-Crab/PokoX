# TASKS.md — Ordered Implementation Ledger

Agent rule: take the first `READY` task whose prerequisites are verified in the actual repository. Keep tasks small. `DONE` requires code plus test/build evidence.

Statuses: `READY`, `BLOCKED`, `IN_PROGRESS`, `DONE`, `DECISION_REQUIRED`.

## Phase 0 — Repository and build contract

### PKX-001 — Inspect and normalize repository baseline
- Status: `DONE`
- Contract: `docs/roadmap/stage-1-foundation.md`
- Depends on: none
- Deliver: inspect existing tree; establish CMake/C++20 conventions, `.gitignore`, formatting, docs, and test entry points without overwriting working code.
- Acceptance:
  - [x] Repository state and toolchain requirements documented.
  - [x] Clean configure/build command is documented and reproducible where toolchain is available.
  - [x] Existing work preserved; no speculative engine feature added.
  - [x] `STATUS.md` records exact observed state.

### PKX-002 — Establish CMake and automated test skeleton
- Status: `DONE`
- Contract: `docs/roadmap/stage-1-foundation.md`
- Depends on: PKX-001
- Deliver: root CMake project, presets where appropriate, CTest registration, minimal test target, and CI workflow if supported by repository.
- Acceptance:
  - [x] Clean configure succeeds on target toolchain.
  - [x] Build succeeds.
  - [x] CTest executes at least one real test.
  - [x] CI uses pinned/reproducible toolchain/dependency setup.

### PKX-003 — Core result/error, logging, and stable ID primitives
- Status: `READY`
- Contract: `docs/03-engine/cpp-engine-foundation.md`
- Depends on: PKX-002
- Deliver: minimal core primitives with explicit contracts and tests.
- Acceptance:
  - [ ] Invalid/error states are explicit and testable.
  - [ ] ID uniqueness/stale-handle behavior is tested as appropriate.
  - [ ] Public APIs have Doxygen.
  - [ ] Unit tests pass.

### PKX-004 — Windows application lifecycle
- Status: `BLOCKED`
- Contract: `docs/10-platforms/windows-and-android.md`
- Depends on: PKX-002, PKX-003
- Deliver: create, run, resize/event-pump, and orderly shutdown of a minimal Windows app.
- Acceptance:
  - [ ] Window lifecycle works on the target Windows toolchain.
  - [ ] Shutdown releases owned resources.
  - [ ] Platform-specific code is isolated.
  - [ ] Manual/test verification is recorded.

## Phase 1 — Renderer proof

### PKX-010 — Pin and integrate renderer dependency
- Status: `BLOCKED`
- Contract: `docs/03-engine/rendering.md`
- Depends on: PKX-004
- Deliver: select and pin a verified Diligent Engine revision; build its minimal sample using the actual toolchain.
- Acceptance:
  - [ ] Exact revision and license recorded.
  - [ ] API usage verified against pinned source/headers.
  - [ ] Clean build succeeds.
  - [ ] Dependency decision recorded if platform/backend choice remains open.

### PKX-011 — Render clear, resize, and present
- Status: `BLOCKED`
- Contract: `docs/03-engine/rendering.md`
- Depends on: PKX-010
- Deliver: renderer initialization, clear/present, resize, and orderly teardown.
- Acceptance:
  - [ ] Frame presents without validation errors.
  - [ ] Resize/recreate path works.
  - [ ] Failure to initialize reports an actionable error.
  - [ ] Lifecycle tests or documented manual checks pass.

### PKX-012 — First visible primitive and camera
- Status: `BLOCKED`
- Contract: `docs/03-engine/rendering.md`
- Depends on: PKX-011
- Deliver: draw a triangle or cube with a basic camera and diagnostic output.
- Acceptance:
  - [ ] Primitive is visible on target.
  - [ ] Resource ownership/lifetime is documented.
  - [ ] Debug and release builds are checked where available.

## Phase 2 — World and local gameplay

### PKX-020 — Entity/component and transform core
- Status: `BLOCKED`
- Contract: `docs/03-engine/world-and-entities.md`
- Depends on: PKX-003, PKX-012
- Deliver: entity handles, components, transforms, and parent-child hierarchy.
- Acceptance:
  - [ ] Invalid/stale handles are rejected.
  - [ ] Parent-child transforms are tested.
  - [ ] Serialization round trip is tested.
  - [ ] World module has no renderer/OS ownership.

### PKX-021 — Scene save/load
- Status: `BLOCKED`
- Contract: `docs/06-content/project-and-package-formats.md`
- Depends on: PKX-020
- Deliver: versioned scene serialization and safe load validation.
- Acceptance:
  - [ ] Round trip preserves supported data.
  - [ ] Unknown/invalid schema behavior is deterministic.
  - [ ] Path traversal and unreasonable sizes/counts are rejected.
  - [ ] Failed loads do not partially corrupt active state.

### PKX-022 — Minimal physics and character controller
- Status: `BLOCKED`
- Contract: `docs/03-engine/physics-and-characters.md`
- Depends on: PKX-020
- Deliver: static collision, raycast/query, trigger, and deterministic basic character movement slice.
- Acceptance:
  - [ ] Collision/query test suite passes.
  - [ ] Tunneling and timestep limitations are documented and tested.
  - [ ] Physics is isolated behind an engine-owned interface.
  - [ ] No claims of production-grade physics without evidence.

### PKX-023 — Asset identity and minimal import
- Status: `BLOCKED`
- Contract: `docs/04-studio/assets-and-prefabs.md`
- Depends on: PKX-021
- Deliver: stable asset IDs, source metadata, validation, and one supported import path.
- Acceptance:
  - [ ] Invalid and oversized inputs rejected.
  - [ ] Content hash/importer version recorded.
  - [ ] Reimport preserves stable references where supported.
  - [ ] Import errors are actionable.

## Phase 3 — Scripting and authoring

### PKX-030 — MUTE language MVP decision and parser prototype
- Status: `BLOCKED`
- Contract: `docs/05-mute/language-design.md`
- Depends on: PKX-003
- Deliver: freeze the smallest language grammar with an executable parser/conformance test.
- Acceptance:
  - [ ] Grammar and diagnostics documented.
  - [ ] Valid/invalid program corpus exists.
  - [ ] No unrestricted host access is exposed.
  - [ ] Language decision is recorded.

### PKX-031 — MUTE bounded runtime
- Status: `BLOCKED`
- Contract: `docs/05-mute/runtime-sandbox-and-errors.md`
- Depends on: PKX-030
- Deliver: execute a tiny program with instruction, memory, stack, and host-call budgets.
- Acceptance:
  - [ ] Budget exhaustion stops execution safely.
  - [ ] Host APIs require explicit capabilities.
  - [ ] Error paths and cancellation are tested.
  - [ ] Runtime cannot directly access OS/filesystem/network/native APIs.

### PKX-032 — First local gameplay event integration
- Status: `BLOCKED`
- Contract: `docs/02-architecture/runtime-lifecycle.md`
- Depends on: PKX-022, PKX-031
- Deliver: invoke a script from one gameplay event in a local scene.
- Acceptance:
  - [ ] Event ordering is specified and tested.
  - [ ] Script failures do not crash the host.
  - [ ] Resource budgets apply per execution.

### PKX-033 — Minimal Studio and Player workflow
- Status: `BLOCKED`
- Contract: `docs/04-studio/studio-overview.md`
- Depends on: PKX-021, PKX-023
- Deliver: create/open project, edit minimal scene, save, and launch it in Player.
- Acceptance:
  - [ ] Save-close-reopen preserves scene.
  - [ ] Missing assets produce clear diagnostics.
  - [ ] Project-relative references work.
  - [ ] Smoke test is reproducible.

## Phase 4 — MVP vertical slice

### PKX-040 — UI Maker MVP
- Status: `BLOCKED`
- Contract: `docs/04-studio/ui-maker.md`
- Depends on: PKX-033
- Deliver: minimal panel/text/button/image layout, properties, preview, and serialization.
- Acceptance:
  - [ ] UI document round-trips.
  - [ ] Basic interaction works in Player.
  - [ ] Layout errors are visible and deterministic.

### PKX-041 — Obstacle-course gameplay slice
- Status: `BLOCKED`
- Contract: `docs/roadmap/first-vertical-slice.md`
- Depends on: PKX-032, PKX-040
- Deliver: floor/platforms, player spawn, movement/jump, hazard, checkpoint, collectible, finish trigger, HUD.
- Acceptance:
  - [ ] All listed gameplay behaviors work end to end.
  - [ ] Project survives save/reopen.
  - [ ] Package validates and launches in Player.
  - [ ] Completion and relaunch are tested.

### PKX-042 — Package build and validation
- Status: `BLOCKED`
- Contract: `docs/06-content/project-and-package-formats.md`
- Depends on: PKX-041
- Deliver: deterministic `.pkx` package build and validation; do not execute scripts during validation.
- Acceptance:
  - [ ] Valid package opens.
  - [ ] Corrupt, incompatible, oversized, and traversal packages fail closed.
  - [ ] Package compatibility/version rules are tested.
  - [ ] Creator-to-Player workflow is reproducible.

## Phase 5 — Compatibility and gated online features

### PKX-050 — Android feasibility and compatibility gate
- Status: `BLOCKED`
- Contract: `docs/10-platforms/windows-and-android.md`
- Depends on: PKX-042
- Deliver: validate actual device/build support before committing to Android runtime scope.
- Acceptance:
  - [ ] Minimum devices/toolchain documented.
  - [ ] Performance and graphics compatibility measured.
  - [ ] Unsupported capabilities fail gracefully.

### PKX-060 — Publishing/accounts/discovery
- Status: `BLOCKED`
- Contract: `docs/07-online/publishing-discovery-and-accounts.md`
- Depends on: PKX-042, security and moderation gate
- Deliver: only after an explicit product decision and operational readiness review.
- Acceptance:
  - [ ] Authn/authz separation and server-side checks.
  - [ ] Immutable published versions and rollback behavior.
  - [ ] Abuse limits, audit logs, and privacy rules tested.

### PKX-070 — Social/multiplayer/chat
- Status: `BLOCKED`
- Contract: `docs/07-online/social-and-multiplayer.md`
- Depends on: PKX-060 and `docs/09-trust-safety/moderation-and-child-safety.md`
- Deliver: no open chat until report/block/moderation/appeal and child-safety operations are ready.
- Acceptance:
  - [ ] Server-authoritative state and rate limits.
  - [ ] Block/privacy rules apply consistently.
  - [ ] Abuse/report/escalation flows are tested.

### PKX-080 — Real-money commerce
- Status: `BLOCKED`
- Contract: `docs/08-marketplace/commerce-and-entitlements.md`
- Depends on: PKX-060, legal/provider/security/support/moderation readiness
- Deliver: do not enable real payments until every gate is explicitly approved.
- Acceptance:
  - [ ] Signed provider events verified idempotently.
  - [ ] Refund/dispute and immutable ledger behavior tested.
  - [ ] Entitlements enforced server-side.
  - [ ] Operational, legal, and support readiness recorded.
