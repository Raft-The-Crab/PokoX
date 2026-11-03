# 35 — Moderation Operations, Capacity and Reviewer Playbook

**Rule:** all new public assets/libraries and every public game release receive human review before distribution. Automation prepares and prioritizes work; it does not auto-approve.

## 1. Review object types

- Individual asset version.
- MUTE library version and dependencies.
- Game release (`.pkx` + matching `.pkB`).
- Thumbnail, screenshot, audio and other listing media.
- Changed release after update.

## 2. Review statuses

`UPLOADED → QUARANTINED → TECHNICAL_CHECK → PENDING_REVIEW → UNDER_REVIEW → APPROVED | REQUIRES_CHANGES | REJECTED`.

Previously approved content may transition to `SUSPENDED` or `REMOVED` after a report, new evidence, policy update or security incident. Every approval references exact hashes and policy version.

## 3. Moderator roles

- Asset Moderator: asset/library review within assigned category.
- Game Moderator: release gameplay and content review.
- Senior Moderator: appeal, escalations and complex policy decisions.
- Child Safety Incident Lead: restricted sensitive incident handling.
- Moderator Administrator: staff roles and security; cannot silently alter past audit records.

A moderator should receive only the permissions necessary for assigned work. All sensitive evidence reads and enforcement actions are audited.

## 4. Review workspace

Poko Mod launches a separate `poko-review-host` using the same pinned Poko Engine libraries as the client and Studio. The Review Runtime loads the exact `.pkx` and `.pkB` submission using mock identities and services. The trusted dashboard retains moderator session credentials; the untrusted game process receives none.

Review tools should include:

- Game viewport: play, pause, restart, camera, screenshot and test spawn.
- Scene hierarchy: inspect objects and locate them in the viewport.
- Asset inspector: model, texture, audio and animation preview.
- Animation timeline: pause/scrub frames and inspect character rig.
- Script inspector: MUTE realm, capabilities, dependencies and static-analysis findings.
- Network inspector: schemas/events observed in the isolated review session.
- Evidence capture: screenshot, permitted short clip, scene snapshot and package hashes.

## 5. Content decision guide

- Sexualized/sexually explicit content is prohibited across all game ages and content labels.
- Child sexualization/exploitation is a critical incident and handled by trained authorized personnel under legal procedure.
- Fictional combat, character deaths, FPS/military themes, horror and bounded stylized blood/gore may be approved depending on context/intensity and PokoX label.
- Graphic realistic mutilation, cruelty/torture-focused presentation, real-world injury footage and sexualized violence are rejected.
- Fictional weapons are not automatically illegal real-world contraband; judge use and context.
- A game's opening area is not sufficient. Check modes, unlockables, procedural content, random events, chat, interactions and server-defined behavior.

## 6. Review procedure

1. Confirm submission ID, creator, version and package hashes.
2. Verify technical checks and dependencies.
3. Launch isolated review session; no production login, economy or network access.
4. Inspect all required content categories.
5. Play representative routes/modes and test declared features.
6. Record checklist findings and evidence references.
7. Select decision and policy reason codes.
8. For changes/rejection, provide actionable non-sensitive feedback.
9. Escalate serious safety incidents; do not circulate illegal material.
10. Submit signed/audited decision.

## 7. Re-review and appeals

Changed package hashes require a new submission. Exact unchanged dependencies can reuse approved hash decisions if policy version is still valid. Appeals should be reviewed by a different moderator where practical. A serious safety restriction remains active during appeal unless senior review explicitly changes it.

## 8. Capacity management

Estimate review minutes by submission type, complexity and risk. Set intake caps based on trained moderator capacity. New creators may receive small quotas; good history can increase throughput but never removes human review. Repeatedly abusive submissions may reduce quotas or suspend publishing. If queue age or staff capacity exceeds thresholds, pause intake or show a queue delay rather than auto-approving.

Priority order: credible immediate child-safety reports; active dangerous releases; high-severity reports; new public release review; routine asset/library review; appeals according to severity and age.

## 9. Reviewer safety

Provide content warnings, short exposure blocks, escalation, two-person review for high-risk cases, staff support and access logs. Ordinary moderators must not download/copy/forward suspected child sexual abuse material. Follow counsel-approved evidence preservation and reporting process.

## 10. Metrics

Track queue size, median/p95 wait, review duration, rework rate, appeal overturn rate, reviewer workload, false-positive/false-negative indicators, report handling age and policy breach recurrence. Avoid productivity quotas that pressure reviewers to rush.
