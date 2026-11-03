# 15 — Publishing, Human Review, and Release Management

**Status:** Proposed baseline  
**Core rule:** No new public asset, library, or game release becomes publicly distributable before required human review.

## 1. Submission types

- Asset submission: model, texture, audio, animation, UI component, template or other reusable asset.
- Library submission: MUTE module/library and dependency set.
- Game release submission: immutable `.pkx`, corresponding `.pkB` when online authority is used, manifest, dependencies and content declarations.
- Profile/avatar item submission: same asset pipeline and policy.

## 2. Submission states

`Draft → Uploaded → Quarantined → TechnicalCheck → PendingReview → InReview → Approved | RequiresChanges | Rejected | Escalated`.

Post-approval states include `Published`, `Suspended`, `Revoked`, `Removed`. Every transition records actor, timestamp, reason and associated content hash.

## 3. Quarantine

Uploads enter private R2 quarantine. They are not publicly fetchable or discoverable. Technical validation checks file signatures, package structure, size, hash, path safety, dependency status, MUTE format, unsupported native payloads and malware indicators where available. Technical checks are triage only; they do not approve content.

## 4. Human review requirement

- Every newly uploaded public asset receives human review before public distribution.
- Every new public game release receives human gameplay/content review before publication.
- New or modified assets require review; exact unchanged previously approved content may reuse an existing approval only when hash, creator/license rights, policy version and context allow it.
- Every game release gets a human review of changed content and overall experience, even when it references approved assets.
- All reviewed decisions bind to immutable submission IDs and content hashes.

## 5. Asset review checklist

Visuals/geometry, texture/decal, animation playback, audio playback, metadata, intended use, rights/license declaration, dependencies, MUTE source/permissions, external links, package integrity and policy classification. Reviewer tooling should preview content without downloading it to a personal device.

## 6. Game review checklist

- Spawn areas, maps, hidden/locked areas and random events.
- Characters, clothing, models, textures and animations.
- Audio/music/voice.
- UI text, images, links and purchase prompts.
- Gameplay mechanics and violence depiction.
- Multiplayer/player-generated interaction.
- MUTE scripts, permissions, event schemas and network behavior.
- Asset dependencies and content labels.
- Offline mode claims and behavior.
- Server package/client package consistency.

Creators must disclose hidden content, procedural content, user-generated content features and dynamic content sources.

## 7. Moderator decisions

- `APPROVED`: publish exact reviewed release.
- `REQUIRES_CHANGES`: fixable issue with actionable explanation.
- `REJECTED`: prohibited or unsuitable content; include policy category and explanation appropriate to disclose.
- `ESCALATED`: sensitive or ambiguous issue requiring senior/trained review.

Appeals should go to a different reviewer where possible. Appeal does not automatically restore content. Sensitive child-safety reports follow restricted incident procedure, not ordinary appeal discussion.

## 8. Versioning and immutable releases

Create a build snapshot before upload. Store manifest digest, all content hashes, `.pkx`/`.pkB` hashes, dependencies, engine/MUTE/protocol versions, policy version and creator declaration. Approval is for those exact hashes. Any changed content creates a new submission. Do not overwrite a published package object in place.

Release channels: Draft, Local Test, Private Beta, Staging and Production. Private testing remains subject to safety rules and reporting. Production updates always pass the review workflow.

## 9. Re-review and reuse

If a game references a previously approved asset hash, the asset itself need not be treated as a new upload, but the game still receives a release review. If policy changes or the asset is reported, approval may be revisited. Revoked asset hashes cannot be included in new releases.

## 10. Capacity management

Human-review capacity is finite. Implement creator submission quotas, queue status, clear estimated ranges only when grounded in current queue data, and pause public submissions when queue exceeds safe capacity. Private editing can continue. Never auto-approve due to backlog. Prioritize child-safety and critical reports separately from routine publishing.

## 11. Review record

Store reviewer ID, submission ID, content hash, policy version, checklist outcomes, decision, violation categories, notes, evidence references, escalation and timestamps. Avoid storing illegal content copies; follow the restricted evidence process in Trust & Safety policy.

## 12. Rollback

A previously approved immutable release may be reactivated only after checking current policy/revocation status. Rollback does not bypass current security checks. If a release is suspended, stop new discovery/download and invalidate future join tickets; already-offline devices may not learn about the change until reconnecting.

## 13. Acceptance tests

- Quarantined object not publicly accessible.
- Public publish blocked without human approval record.
- Modified package hash invalidates approval.
- New asset submission enters manual queue.
- Unchanged approved asset reuse checks matching hash/policy.
- Appeal is tracked separately.
- Queue overload blocks/pauses intake rather than auto-approving.
- Rollback checks current revocation status.
