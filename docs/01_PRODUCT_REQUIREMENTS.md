# 01 — Product Requirements and User Journeys

**Status:** Baseline proposal  
**Primary platform identity:** Free UGC game distribution and creation ecosystem.

## 1. Users and roles

- **Player:** discovers, downloads, installs, launches, updates, and optionally plays games offline.
- **Creator:** builds games and assets in Poko Studio, scripts in MUTE, publishes releases, and may host servers.
- **Server operator:** operates a registered creator-hosted runtime and manages local server infrastructure.
- **Asset/library seller:** submits reusable content to the moderated marketplace and receives Pokoins under platform rules.
- **Moderator:** reviews submissions and reports using Poko Mod.
- **Senior moderator/safety lead:** handles appeals, escalations, and sensitive incidents.
- **Platform administrator:** manages platform configuration, credentials, and enforcement. This is a highly restricted role.

A single account may have multiple roles, but role checks are server-side and every privileged action is audited.

## 2. Player requirements

### Library and install

- Browse approved games and categories.
- View content label, supported platforms, online/offline capability, package size, update date, and creator.
- Download, pause, resume, verify, update, uninstall, and repair a game.
- Reuse shared content-addressed chunks when safe.
- Keep local saves separate from removable package cache.
- Display clear offline/online requirements before install.

### Play

- Launch the installed approved package after signature and integrity verification.
- Support Windows keyboard/mouse/controller and Android touch/controller where game supports them.
- Join a registered session using a short-lived, game-scoped join ticket.
- Continue supported offline gameplay without pretending to be connected to the authoritative server.
- Display service unavailable, session full, update required, and moderation-restricted states clearly.

### Social and safety

MVP social defaults should be conservative: profiles with minimal public data, friends, parties if capacity allows, block, mute, report, and approved quick-chat. Unrestricted voice and direct messaging are deferred. Player reports must reach PokoX independently of the game server.

## 3. Creator requirements

- Create and open `.pk` projects on Windows and Android using the same project schema.
- Build worlds, UI, scripts, assets, materials, and animations.
- Use beginner and advanced UI Maker modes over one document model.
- Test locally and inspect script/runtime diagnostics.
- Configure client/server/shared MUTE realms and declared capabilities.
- Define offline support and prove it with disconnected tests.
- Build `.pkx` and `.pkB` outputs from a reproducible project snapshot.
- Submit immutable release versions for human moderation.
- Review rejection reasons, correct content, resubmit, and appeal decisions.
- Register and revoke creator server instances.
- Inspect aggregated performance and crash metrics without receiving unnecessary personal data.

## 4. Moderator requirements

- Review each new asset submission and each public game release before it becomes public.
- Inspect the exact content hash that was submitted.
- Preview models, textures, audio, animations, UI, scripts, package dependencies, and gameplay.
- Launch the exact `.pkx` and corresponding `.pkB` in an isolated Review Runtime.
- Use synthetic test accounts and isolated game state.
- Assign a content label and record policy version and reasons.
- Approve, request changes, reject, or escalate.
- Handle reports, suspensions, appeals, and post-publication audits.
- Never expose moderator credentials or platform database credentials to reviewed code.

## 5. Developer experience requirements

PokoX should provide documentation, example projects, MUTE API reference, server SDK, package format reference, migration guides, and test lab. Developers should understand quotas and hosting responsibility before publishing.

## 6. Game listing and content labels

Proposed internal labels: `Poko E`, `Poko 10+`, and `Poko 13+`. These are PokoX labels, not formal ESRB/PEGI ratings. Actual labels, parental restrictions, and launch jurisdictions require policy/legal review. Sexualized and sexually explicit material is prohibited across all labels. Fictional combat may be allowed depending on intensity, realism, context, and audience suitability.

## 7. Key user journeys

### Player journey

Discover → review details/content label → download → verify → install → launch → play online or offline depending on game → update → report/block if needed.

### Creator journey

Create `.pk` → build world/UI/scripts → run local tests → build `.pkx` and `.pkB` → technical validation → human review → correct/resubmit if needed → publish immutable release → register server or use PokoX-hosted capacity → monitor health.

### Moderator journey

Authenticate with strong MFA → claim review → inspect metadata and package hashes → open isolated runtime → test game/inspect assets → record evidence and content label → decide → notify creator → audit.

### Creator server journey

Generate registration challenge → generate server key pair → register public key → receive scoped server identity → acquire short-lived credentials → establish session → call scoped Poko Services → heartbeat → rotate/revoke credentials.

## 8. Product non-functional requirements

- Predictable behavior across Windows and Android.
- Clear errors and recovery paths.
- No platform secrets in clients or creator packages.
- No public distribution before moderation approval.
- All economic mutations transactional and idempotent.
- Safe offline failure behavior.
- Privacy by data minimization.
- Accessible UI scaling and remappable controls.
- Logs must avoid secrets and unnecessary personal information.

## 9. MVP acceptance criteria

A release candidate must demonstrate one complete creator-to-player workflow; hash-bound moderation; offline-capable sample game; online sample game with authoritative action validation; creator server revocation; player report; local save; and successful restoration of a test database backup.
