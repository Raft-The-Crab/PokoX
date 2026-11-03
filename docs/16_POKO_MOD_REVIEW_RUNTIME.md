# 16 — Poko Mod Review Runtime

**Status:** Proposed baseline  
**Key decision:** Poko Mod uses shared Poko engine components to execute submitted games, but untrusted content runs outside the privileged Poko Mod process.

## 1. Purpose

Moderators must inspect the actual submitted player package and game-authority package rather than relying only on screenshots or an editable project. The Review Runtime provides a controlled way to play, inspect and test a release.

## 2. Shared engine modules

Reuse the same pinned versions of world loader, renderer, physics, animation, audio, runtime UI, asset system, MUTE client runtime and package reader used by PokoX. The Poko Mod host provides review controls and evidence workflows. Do not fork a separate game engine for moderation.

## 3. Process architecture

```text
Poko Mod (trusted Rust/egui host)
  - Moderator auth/session
  - Review queue and decision UI
  - Audit/evidence references
  - No creator code execution
          |
          | narrow authenticated local IPC
          v
Poko Review Host (restricted process)
  - Shared Poko runtime
  - Exact submitted .pkx
  - Matching .pkB / authority harness
  - Synthetic test accounts
  - Temporary local storage
  - Restricted networking/filesystem
```

A separate process is a containment boundary, not a complete sandbox by itself. Use restricted Windows process token, Job Object resource limits and AppContainer or other OS-level restrictions where feasible. For high-risk package/parser testing, prefer disposable VM isolation if hardware permits. Creator code must not run with moderator credentials or access production databases.

## 4. Exact package loading

Review record identifies immutable `.pkx` and `.pkB` hashes. Runtime verifies platform signature and all content hashes, then launches the exact artifacts. Editable `.pk` may be examined for additional source context but is not the release artifact under approval.

## 5. Review workspace

- Game viewport: play, pause, restart, fullscreen, camera/spectator, spawn selection.
- Scene inspector: read-only hierarchy/object properties and asset references.
- Content inspector: model/texture/audio/animation preview.
- Script inspector: MUTE source when permitted, realm, capabilities, dependencies, static analysis.
- Network inspector: review-session messages and schema failures, with sensitive data redacted.
- Authority inspector: synthetic player state and validation decisions.
- Performance panel: frame time, memory, asset load and runtime warnings.
- Decision panel: checklist, label, reasons, evidence and action.

## 6. Review session setup

1. Claim review item.
2. Resolve exact immutable package and dependencies.
3. Validate hashes/signature.
4. Create temporary sandbox directory and synthetic accounts.
5. Start isolated authority process for `.pkB` when applicable.
6. Start review client process for `.pkx`.
7. Disable production Pokoins, production purchases and real-player connectivity.
8. Capture review findings.
9. Stop all processes and erase temporary non-evidence state.
10. Persist decision/evidence references in trusted Poko Mod service.

## 7. Multiplayer tests

Allow multiple synthetic clients connected to isolated review authority. Test combat, chat, trading, teams, player interaction and game-specific mechanics. Never connect test clients to production sessions. Network isolation must be enforced at OS/container policy, not merely by a UI toggle.

## 8. Review inspector privilege separation

The runtime may expose read-only diagnostic events to Poko Mod through a narrow typed IPC contract. The game cannot call Poko Mod functions, approve itself, read moderator credentials, access report queues or change review state. Evidence capture must be initiated by trusted host controls.

## 9. Security and moderation limitations

A community-hosted operator can modify their production server after approval. The Review Runtime tests the submitted `.pkB` under official runtime conditions; it cannot prove the creator's live host executes it unchanged. PokoX must enforce client/content manifests and platform APIs, and classify community-hosted behavior as untrusted. PokoX-hosted sessions permit stronger process and binary control.

## 10. Acceptance tests

- Poko Mod launches `.pkx` and matching `.pkB` in review process.
- Malformed package crashes only review process, not Poko Mod host.
- Review process cannot read moderator token or production secrets.
- Review session has no production network access except narrowly required review services.
- Synthetic players can test multiplayer.
- Evidence binds to submission ID and package hashes.
- Process termination cleans temporary files and child processes.
- Poko Mod can continue operating after review runtime crash.

## 11. Final shared-engine contract

Poko Mod reuses the same pinned Poko Engine Core and client runtime libraries as PokoX and Poko Studio, but not the same trust process. The trusted Rust/egui moderator dashboard launches a separate `poko-review-host` process. That process loads the exact submitted `.pkx` and its matching `.pkB` in a disposable, restricted review environment. The review host reports viewport frames and approved diagnostic data to the dashboard through a narrow local authenticated IPC contract.

The Review Runtime must not inherit moderator session tokens, production API credentials, database credentials or general filesystem access. It uses synthetic review identities and mock economy/purchase services. It must not create real Pokoins, modify real player accounts, publish releases, or connect to public game sessions. A child process alone is not a complete security boundary; use Windows restricted tokens, Job Objects, ACLs and, for high-risk packages, a disposable VM. If full embedding conflicts with isolation, prefer a separate review window over executing untrusted code in the moderator UI process.

Review evidence references the submission ID, release ID, exact `.pkx`/`.pkB` hashes, engine build and review policy version. Poko Mod may inspect the `.pk` source project only as supplemental evidence; approval applies to the immutable published package hashes.

## 12. Acceptance additions

- A crash in the review process does not crash the Poko Mod dashboard.
- A malicious package cannot read moderator credentials or production data.
- Review environment has no production purchase, Pokoins or moderation-admin capability.
- Exact submitted client/backend package hashes are displayed and recorded.
- Review session cleanup preserves only explicitly retained evidence/audit metadata.
