# 00 — PokoX Master Plan

**Planning baseline:** 1.0 (final consolidated planning release)  
**Prepared:** 2026-11-03  
**Product stage:** Pre-production architecture; implementation has not yet been validated by prototypes.  
**Budget objective:** $0 for development and initial prototype/closed-alpha infrastructure, subject to provider eligibility and quotas. This is not a promise of unlimited free public hosting.

## 1. Product definition

PokoX is a free-to-use game distribution and creation platform. Its player experience resembles a downloadable game launcher/library; its creator ecosystem resembles a user-generated game platform; and its multiplayer hosting model allows creators to operate their own dedicated servers. PokoX supplies a shared engine, MUTE scripting language, creator tools, packages, services, moderation, discovery and a virtual economy.

The platform has three applications:

- **PokoX Client** — Windows 11+ and Android 11+ game library, downloading, installation, playing, social and offline runtime.
- **Poko Studio** — Windows 11+ and Android 11+ creation environment, world editor, UI Maker, MUTE editor, asset pipeline, testing, server configuration and publishing.
- **Poko Mod** — Windows 11+ moderator console with reports, publishing review, live diagnostics and an isolated shared-engine Review Runtime.

## 2. Product identity

PokoX combines:

- **Game-launcher/distribution model:** installable game packages, local library, updates, offline play where supported, discovery and version management.
- **UGC creator model:** games, assets, libraries, editor, scripting and marketplace.
- **Community hosting model:** creator-operated dedicated servers using the official headless runtime and scoped SDK.
- **PokoX-specific systems:** MUTE, `.pk`/`.pkx`/`.pkB`, Poko Review Runtime, Pokoins, human-reviewed publishing and capability-restricted Poko Services.

PokoX is not a game streaming service. Players download game assets/packages and execute the client runtime locally.

## 3. Product pillars

1. **Play:** install, update, discover, play online or offline where supported.
2. **Create:** build worlds, interfaces, scripts, assets and multiplayer experiences.
3. **Host:** operate community-hosted game servers or request limited PokoX-hosted capacity.
4. **Protect:** moderation, anti-cheat, child-conscious defaults, package security and account protection.
5. **Trade and share:** approved assets, libraries and developer-created content using Pokoins, subject to the marketplace policy.

## 4. Non-negotiable system invariants

- Client devices are untrusted. Client code is inspectable and modifiable.
- Community-hosted server operators are untrusted for platform-wide claims. Registration proves identity/credential possession, not honest execution.
- PokoX owns account identity, Pokoins, platform ownership, approvals, moderation, reporting and platform credentials.
- Creator servers own game-specific authoritative state only for their own sessions and games.
- MUTE is feature-rich but capability-restricted; client/server/shared/Studio realms have distinct APIs.
- Server-only MUTE source and secrets never ship in `.pkx`.
- Online multiplayer state that affects fairness or competitive outcomes is server-authoritative; offline saves are untrusted.
- Published package manifests and content references are signed; content hashes are verified before load.
- Every new public asset version and public game release is human-reviewed before publication. Unchanged dependencies may reuse exact hash-bound approval only when policy permits.
- Fictional combat and bounded stylized violence can be allowed under a content label. Sexualized/sexually explicit content is prohibited platform-wide.
- Poko Mod never executes untrusted creator content in its privileged dashboard process.
- Free-tier infrastructure is for prototyping/limited alpha; availability and capacity are never guaranteed.
- Provider substitution must not require changes to game/package semantics.

## 5. Authoritative ownership map

| Data or decision | Authority |
|---|---|
| Platform identity and account status | PokoX backend / Firebase identity mapping |
| Pokoins and marketplace ownership | PokoX transactional backend |
| Global reports, moderation and approvals | PokoX backend |
| Game-specific session state | Game authority for that game/session |
| Game-specific persistent data | Poko DataStore namespace, authorized by PokoX |
| Local offline progression | Player device; unverified until validated |
| Public package approval | PokoX review record bound to exact hashes and policy version |
| Community server code execution | Creator's host; not independently trusted by PokoX |

## 6. First end-to-end vertical slice

Before building all applications, prove a Windows slice:

1. MSYS2 UCRT64 + CMake + Ninja build.
2. SDL2 creates a window and receives input.
3. Diligent Core OpenGL backend renders a triangle.
4. Shared C++ engine loads one test world and character.
5. Minimal custom physics supports floor collision and movement.
6. Minimal MUTE client script executes under a strict budget.
7. A `.pkx`-like development package is parsed, hash-verified and loaded.
8. Two clients exchange a schema-defined input/state message with a headless C++ authority.
9. Authority rejects an impossible action.
10. Offline mode launches the same local package and stores local-only progress.
11. Poko Mod starts a separate Review Runtime process and loads the exact submitted package in a no-network test.

## 7. Release definitions

### Prototype
One local game, engine vertical slice, MUTE interpreter subset, package reader, offline launch and local multiplayer proof. No public user uploads or economy.

### Closed alpha
Invited creators, private publishing submissions, human review, restricted player count, basic creator hosting, reports/blocking, local library and explicit infrastructure limits. Pokoins may remain disabled.

### Public MVP
Approved game/asset publishing, discovery, installation, offline-ready support, creator server SDK, basic DataStore, Poko Mod, safety operations, backups and incident response. Marketplace and Pokoins activate only after transaction tests and moderation capacity gates.

### Expanded platform
Broader social features, richer UI Maker, Android Studio completion, marketplace subscriptions, advanced analytics, custom server integrations and higher-capacity hosting after funding/eligibility.

## 8. Initial-release non-goals

- Unlimited or guaranteed 24/7 free hosting.
- Perfect anti-cheat, unbreakable DRM or impossible-to-extract client assets.
- Arbitrary native MUTE libraries.
- Unrestricted client filesystem, process, socket or operating-system access.
- Voice chat, unrestricted private messaging, or arbitrary in-game image/audio uploads.
- Pokoins cash-out or real-money conversion.
- Unmoderated public publishing.
- Full collaborative editing and advanced physics such as fluids/destruction.

## 9. Dependencies and build order

Engine feasibility → engine/world → MUTE and sandbox → PPC/package formats → authority/protocol → creator server SDK → Studio MVP → backend/publishing → Poko Mod review → safety operations → player client/offline → services/social → marketplace/economy → Android completion/public release.

Some UI work can proceed in parallel, but no public publishing should precede package validation and staffed human review.

## 10. Project completion criteria

The plan is complete as a planning baseline. Implementation is complete only when milestone acceptance tests pass. See `24_ROADMAP_MILESTONES.md`, `33_IMPLEMENTATION_BACKLOG.md`, `34_DEPLOYMENT_RUNBOOK.md`, and the ADRs for execution requirements.
