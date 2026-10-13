# Initial Sequenced Task Backlog

Create issues from these tasks with owner, estimate, dependencies, acceptance criteria and test evidence.

## Epic A — Establish repository truth
- [ ] A1 Record Poko/Mute commit hashes and working-tree status.
- [ ] A2 Document clean Windows build.
- [ ] A3 Run tests and capture failures.
- [ ] A4 Inventory dependencies/licenses/build flags.
- [ ] A5 Mark subsystems Verified/Partial/Absent/Broken.
- [ ] A6 Add CI for current working targets.

## Epic B — Rust UI and engine experiment
- [ ] B1 Minimal Rust library with one exported function/error code.
- [ ] B2 Stable C header/ABI.
- [ ] B3 Minimal EGUI + eframe application for Windows.
- [ ] B4 Minimal Kotlin + JNI bridge to Rust engine for Android.
- [ ] B5 Invalid arguments, repeated calls, shutdown and error tests.
- [ ] B6 Record go/no-go migration decision.

## Epic C — Engine minimum
- [ ] C1 Stable IDs/handle rules.
- [ ] C2 Lifecycle and transform ownership.
- [ ] C3 Workspace/Model/Part test scene.
- [ ] C4 One metadata-backed type/property/function.
- [ ] C5 Minimal renderer test.
- [ ] C6 Logging/profiling baseline.
- [ ] C7 Asset load/unload test.

## Epic D — Mute minimum
- [ ] D1 Grammar and syntax spec.
- [ ] D2 Lexer tests.
- [ ] D3 Parser/AST/source-span tests.
- [ ] D4 Resolver/semantic checks.
- [ ] D5 Versioned bytecode.
- [ ] D6 Verifier.
- [ ] D7 VM limits.
- [ ] D8 Metadata-backed engine API.
- [ ] D9 Invalid-bytecode fuzzing.
- [ ] D10 Compile-time benchmark fixture.

## Epic E — Physics correctness
- [ ] E1 Units/timestep/tolerance spec.
- [ ] E2 Headless math/transform tests.
- [ ] E3 One authoritative body transform/state.
- [ ] E4 Broadphase proxy lifecycle tests.
- [ ] E5 Raycast point/normal/distance tests.
- [ ] E6 Manifold lifecycle tests.
- [ ] E7 Working solver response; no silent no-op paths.
- [ ] E8 Trigger/body-destruction tests.
- [ ] E9 Reference comparison and fixed-scene benchmark.

## Epic F — Studio vertical slice
- [ ] F1 Create/open project.
- [ ] F2 Create Part and edit transform.
- [ ] F3 Save/reopen scene.
- [ ] F4 Attach script and show diagnostics.
- [ ] F5 Playtest/stop.
- [ ] F6 Package and validate local build.

## Epic G — Backend/session slice
- [ ] G1 Account/session schema.
- [ ] G2 Sign-in/revoke.
- [ ] G3 Experience metadata/version pointer.
- [ ] G4 Secure package upload/download.
- [ ] G5 Authority handshake/protocol codec.
- [ ] G6 Two-client join/leave.
- [ ] G7 Report/mute and audit.
- [ ] G8 Staged load tests.

## Dependencies
A precedes rewrites. B precedes Rust migration commitment. C metadata precedes deep Mute/Studio integration. D verifier/limits precede untrusted published scripts. E correctness gates precede production physics claims. F precedes creator marketplace. G precedes public multiplayer.
