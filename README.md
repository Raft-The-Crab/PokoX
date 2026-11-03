# PokoX — Final Project Plan

**Planning release:** 1.0  
**Date:** 2026-11-03  
**State:** Consolidated planning baseline; not a claim that software has been implemented or security-certified.  
**Target:** Free-to-use platform; $0 development/prototype budget; Windows 11+ and Android 11+; Poko Mod on Windows 11+.

PokoX is a downloadable UGC game platform. It combines a Steam-like installed-game library and offline-capable launcher, a Roblox-like creation ecosystem, and a Minecraft-inspired community-hosted dedicated-server model. It has a shared C++ engine, MUTE programming language, three Poko package formats, human-reviewed publishing, Poko Mod, Poko Services and a Pokoins economy.

## What is in this package

- **37 technical documents** in `docs/`.
- **9 architecture decision records** in `adr/`.
- `README.md`, `CHANGELOG.md`, `REFERENCES.md`, and `MANIFEST.md`.
- A file manifest with SHA-256 checksums.

## Reading order

### Product and architecture
1. `docs/00_MASTER_PLAN.md`
2. `docs/01_PRODUCT_REQUIREMENTS.md`
3. `docs/02_SYSTEM_ARCHITECTURE.md`
4. `docs/25_DECISIONS_GLOSSARY_RISKS.md`
5. `docs/32_FINAL_SYSTEM_CONTRACTS.md`

### Engine and scripting
6. `docs/03_ENGINE_ARCHITECTURE.md`
7. `docs/04_RENDERING_INPUT_PLATFORM.md`
8. `docs/05_WORLD_ECS_PHYSICS.md`
9. `docs/26_BUILD_ENVIRONMENT_AND_CMAKE.md`
10. `docs/06_MUTE_LANGUAGE_SPEC.md`
11. `docs/07_MUTE_SANDBOX_SECURITY.md`

### Packages, networking and hosting
12. `docs/11_POKO_PACKAGE_FORMATS.md`
13. `docs/08_NETWORK_BITSTREAM_PROTOCOL.md`
14. `docs/09_GAME_AUTHORITY_HOSTING.md`
15. `docs/10_CREATOR_SERVER_SDK.md`
16. `docs/28_RELEASE_VERSIONING_COMPATIBILITY.md`

### Backend and infrastructure
17. `docs/12_BACKEND_DATA_SERVICES.md`
18. `docs/27_DATABASE_SCHEMA_API_CONTRACTS.md`
19. `docs/29_RILAVEK_UPLOAD_INGESTION.md`
20. `docs/30_HUGGING_FACE_TWO_ACCOUNT_DEPLOYMENT.md`
21. `docs/31_PROVIDER_RESOURCE_ALLOCATION.md`

### Applications and platform features
22. `docs/13_POKO_STUDIO_SPEC.md`
23. `docs/14_PLAYER_CLIENT_OFFLINE.md`
24. `docs/16_POKO_MOD_REVIEW_RUNTIME.md`
25. `docs/20_SOCIAL_DISCOVERY_PROFILES.md`
26. `docs/21_ANALYTICS_LIVEOPS.md`
27. `docs/33_IMPLEMENTATION_BACKLOG.md`

### Moderation, security and operations
28. `docs/15_PUBLISHING_REVIEW_PIPELINE.md`
29. `docs/17_TRUST_SAFETY_MODERATION.md`
30. `docs/18_SECURITY_ANTICHEAT.md`
31. `docs/19_POKOINS_MARKETPLACE.md`
32. `docs/35_MODERATION_OPERATIONS.md`
33. `docs/34_DEPLOYMENT_RUNBOOK.md`
34. `docs/23_TESTING_QA_RELEASE.md`
35. `docs/22_INFRASTRUCTURE_DEVOPS.md`
36. `docs/24_ROADMAP_MILESTONES.md`

Then read `REFERENCES.md` and all ADR files.

## Final architecture decisions

- Windows engine stack: C++ / Diligent Core / OpenGL / SDL2 / custom physics / CMake / MSYS2 UCRT64.
- Android renderer target: Diligent OpenGL ES backend; Kotlin application UI; C++ engine via JNI/C ABI boundary.
- Windows application UI: Rust + egui.
- Main platform backend: Go modular monolith.
- Edge layer: TypeScript Cloudflare Workers, used for lightweight gateway work, not the only authorization layer.
- Identity: Firebase Authentication mapped to internal PokoX user IDs.
- Primary transactional database: one authoritative PostgreSQL deployment at a time.
- Cache: Valkey for disposable/ephemeral state.
- Object storage: Cloudflare R2, 10 GB-month Standard free storage per account (not per bucket), subject to operations quota.
- Rilavek: optional TUS/resumable file-ingestion bridge into R2; not storage, not CDN, not database. Free plan advertises 10 GB monthly transfer and 7-day transfer logs.
- Hugging Face: desired two-account separation (Account A Go platform API, Account B C++ authority). Current official docs require paid PRO for personal compute-backed Docker Spaces or Team/Enterprise for organizations; two free accounts do not remove this. Keep this deployment conditional on both accounts independently having eligible compute access. Free Spaces sleep when unused; 50 GB disk is non-persistent. Do not rely on these Spaces for always-on production.
- Game packages: `.pk` editable project, `.pkx` player package, `.pkB` backend package. Shared PPC container, signed manifest, content hashes and chunked assets.
- MUTE: dynamic runtime with optional static checking; Lua-style blocks ending in `end`; 1-based arrays; interpreter first, bytecode VM after; no native libraries initially; client/server/shared/Studio realms; capability restrictions and resource budgets.
- Game hosting: community-hosted is default scalable model; PokoX-hosted capacity limited; local development mode distinct.
- Offline: `OFFLINE_READY`, `HYBRID`, `ONLINE_REQUIRED`; local saves are untrusted; Pokoins and verified platform rewards cannot be generated offline.
- Publishing: human review required for each new public game release and each new public asset/library version; changed content is resubmitted; exact approved hashes may reuse approval when policy allows.
- Moderation policy: all-ages platform; sexualized/sexually explicit material prohibited; fictional violence and bounded stylized gore may be permitted by content label and human review.
- Poko Mod: Rust/egui control plane; isolated Review Runtime process built from the shared engine; exact `.pkx` + matching `.pkB` reviewed.
- Economy: Pokoins only virtual at launch; weekly earning cap proposal 1,500 PK, adjusted only after simulation; seller receives 85%, 15% platform fee; double-entry/ledger-style immutable transactions and idempotency.

## Status language

- **DECIDED:** user-confirmed or selected as this planning baseline.
- **DEFAULT:** recommended implementation default; may change through ADR.
- **CONDITIONAL:** valid only after provider/account/technical eligibility is demonstrated.
- **OPEN:** requires prototype, measurement or legal review; do not hard-code yet.
- **DEFERRED:** intentionally not part of the initial release.

`MUST`, `MUST NOT`, `SHOULD`, and `MAY` are used as engineering requirement terms.

## How to use this plan

Import `docs/` and `adr/` into the PokoX Git repository. Implement one milestone at a time. Keep implementation issues and test results in project tracking. If a technical decision changes, update the relevant document and add an ADR. Do not silently modify package, protocol, database or permission contracts. Verify provider plans again immediately before deployment because free allowances and eligibility can change.
