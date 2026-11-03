# 32 — Final System Contracts and Cross-Module Invariants

This document summarizes the boundaries that every module must preserve. If another specification conflicts with this contract, record an ADR and update every affected interface before implementation.

## 1. Applications

| Application | UI technology | Shared technology | Privilege |
|---|---|---|---|
| PokoX Client | Rust/egui Windows shell; Kotlin Android shell | C++ engine/runtime | Player/client, untrusted |
| Poko Studio | Rust/egui Windows; Kotlin Android | C++ engine, project model, MUTE tooling | Creator, project owner |
| Poko Mod | Rust/egui Windows | C++ shared engine inside separate Review Runtime | Moderator control plane |

The shared engine does not imply shared privileges or shared process execution.

## 2. Engine contracts

- C++ engine is built with CMake.
- Diligent Core provides graphics abstraction; Windows initial backend OpenGL, Android initial backend OpenGL ES.
- SDL2 owns window/input/event integration. Exactly one component owns creation and currentness of the GL context for each surface; validate integration in M0.
- Physics is custom, incremental and exposed behind a replaceable interface.
- World is hybrid ECS + hierarchy + stable handles.
- Frontends call a versioned C ABI; no C++ STL/exception/ownership types cross ABI.
- Kotlin reaches native engine through JNI wrapper around the C ABI.

## 3. MUTE contracts

- Lua-style block syntax with `end`.
- Dynamic runtime with optional static checking; 1-based arrays.
- Interpreter first, bytecode VM later.
- Client, Server, Shared and Studio realms.
- APIs are injected by realm and capability; scripts cannot select/grant their own realm.
- No native library loading in initial release.
- No unrestricted OS file/process/socket/memory access.
- Instruction, memory, call-depth, event, task and network budgets.
- Client scripts are treated as public/inspectable. Sensitive server source never goes to client.

## 4. Packages

- `.pk`: editable source project and backup.
- `.pkx`: player-facing approved client experience.
- `.pkB`: server-authority package.
- A common PPC container handles header, manifest, file index, content chunks and signature metadata.
- SHA-256 identifies content; Ed25519 signs canonical release manifest; Zstandard compression; canonical CBOR manifest are proposed defaults.
- Cryptographic details require test vectors and an independent implementation review before release.
- Client validates platform signature and hashes; creator cannot self-sign a platform approval.
- Package parser runs with strict bounds and fuzz tests.

## 5. Networking and authority

- Schema-defined Bitstream is message serialization, not encryption.
- Use authenticated encrypted transport; transport choice remains a prototype gate.
- Client sends input/intent; server determines canonical online gameplay state.
- Client prediction is reconciled with authority snapshots/events.
- Static world content is downloaded; dynamic state is selectively replicated.
- Creator servers are untrusted operators; PokoX services enforce scope independently.
- Pokoins and account-level actions never derive solely from creator-hosted server claims.

## 6. Platform data

- Firebase UID maps to immutable internal PokoX UserID.
- PostgreSQL is the sole transactional source of truth at a time.
- Valkey is disposable cache/ephemeral data.
- D1 is rebuildable edge read/config data only, if used.
- Poko DataStore is game-scoped and accessed only through authorized backend APIs.
- Local saves are untrusted and cannot mint Pokoins.
- Pokoins use immutable ledger entries, atomic transaction and idempotency.

## 7. Hosting and trust

- Local development, community-hosted and PokoX-hosted are distinct modes.
- Server credentials are unique per instance, key-bound, short-lived and revocable.
- Credential proves identity/possession only, not faithful server execution.
- Platform account/economy/moderation capabilities are never exposed to creator server.
- Community host may access player IP at the network layer; disclose it. Relay is deferred.

## 8. Publishing and safety

- Public asset/library versions and game releases are quarantined until human-reviewed.
- Approval binds exact content hashes and policy version.
- New changed content requires review; exact unchanged approved dependencies may reuse approval when policy allows.
- Sexualized/sexually explicit content prohibited across platform.
- Fictional violence and bounded stylized gore may be approved according to content label and human review; graphic realistic cruelty/torture/real-world harm prohibited.
- Reports bypass creator servers and go to PokoX.
- Poko Mod launches untrusted review packages in isolated Review Runtime, not dashboard process.

## 9. Offline

- Game modes: OFFLINE_READY, HYBRID, ONLINE_REQUIRED.
- All required offline assets must be included or already cached.
- Offline access freshness policy is configurable; revocation cannot reach a disconnected device immediately.
- Local achievements are unverified; Pokoins cannot be earned/spent offline.
- Save sync uses versioning/conditional updates and declared merge rules.

## 10. Infrastructure

- R2 free storage allowance is account-wide 10 GB-month Standard, not per bucket.
- Rilavek is optional resumable ingestion to R2; 10 GB monthly transfer is not storage.
- Hugging Face two-account topology is conditional on both Docker compute eligibility; two free accounts do not bypass paid-plan requirement.
- No provider is assumed always-on or durable solely because it has a free tier.
