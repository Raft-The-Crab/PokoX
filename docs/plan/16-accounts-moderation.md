# Accounts, Safety, and Moderation

## Identity
- [ ] Immutable internal account ID and username/display-name policy.
- [ ] Secure authentication and recovery.
- [ ] Session rotation/revocation.
- [ ] Auth rate limits and abuse detection.
- [ ] Review privacy/deletion obligations for launch markets.

## Enforcement
Define warning, content removal, chat mute, temporary suspension, permanent ban, experience restriction and asset restriction. Store reason code, evidence reference, actor, timestamps, expiry and appeal state. Enforce server-side across joining/publishing; UI hiding is not enforcement.

## Reports/cases
- [ ] Reports for users, chat, experiences, assets and relevant transactions.
- [ ] Queue, priority, assignment, notes and controlled evidence access.
- [ ] Moderator/admin roles with least privilege.
- [ ] Immutable audit trail for privileged actions.
- [ ] Temporary restrictions expire consistently.
- [ ] Appeals and review permissions.
- [ ] Emergency access has a reason and audit record.

## Chat safety
- [ ] Block/mute, spam controls, rate limits and reporting.
- [ ] Define retention and authorized access.
- [ ] DO may coordinate chat connections; backend remains source of long-term policy/enforcement.

## Gate
Report → review → authorized action → audit → appeal → enforcement works across client and services.
