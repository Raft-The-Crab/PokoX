# 14 — PokoX Player Client, Library, Downloads, and Offline Play

**Status:** Proposed baseline  
**Product model:** Downloadable game library, similar to a game launcher/storefront.  
**Targets:** Windows 11+ and Android 11+.

## 1. Main client responsibilities

- Account login and profile.
- Game discovery and store pages.
- Installed game library.
- Download/install/update/verify/uninstall/repair.
- Game launch and runtime lifecycle.
- Session discovery/join.
- Friends/party UI as released.
- Local saves and offline runtime.
- Report/block/mute features.
- Download storage settings and parental/privacy settings.

## 2. Game modes

Each game declares one of:

- `OFFLINE_READY`: core experience runs locally after installation; online features are optional.
- `HYBRID`: a defined offline subset and online features.
- `ONLINE_REQUIRED`: requires active connection/session.

The developer declaration is validated by build checks and human review. A manifest flag alone does not prove offline compatibility.

## 3. Offline runtime

Uses the shared engine, client MUTE realm, local world/physics/UI/audio and local storage. It has no authority to grant Pokoins, platform inventory, verified platform achievements, cloud ownership, or multiplayer authoritative state. Online-only APIs return clear offline errors or invoke declared fallback behavior.

## 4. Download manager

Requirements: resumable downloads, checksum verification, atomic install, update/repair, shared content-addressed chunk cache, storage usage reporting, Wi-Fi-only option, concurrency limit, pause/resume, retry/backoff and cleanup. Package cache and player save data must be separate. Removing one game must not delete shared chunks used by another installed game.

## 5. Local library

Show installed size, required free space, supported device/controls, package version, last update, offline mode, content label, server availability and update state. Allow user-selected Windows library directory; use application-managed Android storage and platform-compliant storage APIs.

## 6. Local saves

Local save data is untrusted because the device owner can modify it. Store within per-game sandboxed app data, validate schema and size, use checksums for corruption detection, and separate save migration from game package updates. Encryption at rest may protect privacy but does not make local state tamper-proof against device owner.

## 7. Cloud sync

When online: authenticate, fetch cloud revision, compare local pending operations, apply game-defined merge policy within platform constraints, write with revision/conditional update, then mark local operations synchronized. Avoid blind last-write-wins for inventories/currency-like progression. Provide conflict policy per schema: replace for settings, merge for independent fields, or explicit conflict resolution for ambiguous progress.

## 8. Offline access and revocation

Signed release manifests allow offline signature verification. Cache current approval/revocation metadata on sync. Offline devices cannot know about new revocations until reconnecting. Do not claim instant remote kill-switch. Because PokoX is free, avoid treating offline tickets as punitive DRM. A configurable freshness/grace policy may be applied for safety-sensitive content, but it must preserve local saves and disclose behavior clearly. High-risk revoked content is blocked as soon as the client learns of revocation.

## 9. Offline achievements and currency

Local achievements can be recorded as `LOCAL_UNVERIFIED`. On reconnect, they may be submitted for evaluation, but a modified client/local save is not sufficient proof for competitive or platform rewards. Pokoins cannot be minted or spent offline. Game-specific offline progress may sync if developer-defined validation accepts it, but community server claims are not automatically trustworthy.

## 10. Session loss

- Online-required: show disconnect and reconnect path; do not silently run fake server state.
- Hybrid: transition to local mode only if the game explicitly defines a safe offline transition and checkpoints state.
- Offline-ready: continue local simulation.

A network disconnect must not cause duplicate purchases, item grants or transactions after reconnect.

## 11. UX requirements

Game details must show download size, installed size estimate, offline capability, online-only features, content label, device compatibility and last update. Download errors must be recoverable. Do not delete saves when uninstalling game assets unless user explicitly chooses save deletion.

## 12. Acceptance tests

- Install and verify a package with no missing chunks.
- Launch offline-ready package without internet.
- Offline-required package gives clear reason.
- Offline save persists after app restart.
- Shared asset cache reference counts remain correct.
- Corrupt chunk is detected and repaired.
- Local save tampering does not grant Pokoins.
- Cloud sync handles conflict and retry idempotently.
- App handles Android suspend/resume and storage pressure.
