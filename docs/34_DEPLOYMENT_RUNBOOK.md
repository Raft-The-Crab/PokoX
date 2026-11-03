# 34 — Deployment, Operations, Backup and Incident Runbook

This document defines how the first deployments are operated. It deliberately distinguishes development, closed alpha and production.

## 1. Environments

- **Local:** developer workstations; synthetic accounts/data.
- **CI:** ephemeral build/test workers; no production secrets.
- **Staging:** separate Firebase project, DB, R2 bucket/prefix and signing keys; fake users and economy.
- **Closed alpha:** invited users, limited games/servers and explicit best-effort availability.
- **Production:** not enabled until compute, moderation, legal and recovery gates pass.

Never share signing private keys, Firebase admin credentials, DB credentials or moderator secrets between environments.

## 2. Deployment artifacts

- Go backend container/image with immutable build identifier.
- C++ authority runtime image/binary with protocol compatibility ID.
- PokoX client and Studio artifacts for Windows/Android.
- Poko Mod and Review Host artifacts.
- Database migration bundle.
- Package signing public-key set and protected private-key reference.
- Software bill of materials and dependency/license inventory.

Build once and promote the same artifact between environments. Do not rebuild from a floating dependency set for production.

## 3. Deployment sequence

1. Check provider quota/eligibility and status.
2. Verify backups and last restore drill.
3. Apply backward-compatible database migration.
4. Deploy Go API with health check.
5. Deploy Workers changes after origin authorization is ready.
6. Deploy authority runtime and verify heartbeat/session registration.
7. Run synthetic login, catalog, package-download and join-session smoke tests.
8. Enable new routes/feature flags gradually.
9. Monitor error rates, quota, memory, connection count and queue depth.
10. Record release version, operator, timestamp and rollback target.

## 4. Rollback

- Application rollback uses prior immutable artifact.
- Database migrations should be expand/contract; destructive schema rollback is not assumed.
- Package release rollback selects a previous still-approved immutable release; recheck current policy/revocation status first.
- Credential compromise requires rotation/revocation, not application rollback alone.
- Economy ledger is never edited to fake rollback. Use compensating transactions.

## 5. Backup and restore

Critical records: account mappings, Pokoins ledger, marketplace ownership, game/release metadata, moderation decisions, server registration metadata and DataStore records. R2 critical objects require inventory/manifests and independent copies or reproducible source backup where feasible.

Minimum procedure:

- Encrypted logical PostgreSQL backup on a schedule suitable for data loss tolerance.
- Keep multiple recovery points in a separate credential/storage boundary.
- Record backup checksum, size, timestamp and schema version.
- Periodically restore to isolated staging and run integrity queries.
- Confirm ledger balances, ownership rows, moderation references and package hashes.
- Never consider backup successful until restore test passes.

## 6. Incident severity

| Severity | Example | Response |
|---|---|---|
| SEV-1 | Platform credential leak, Pokoins ledger compromise, active child-safety emergency | Restrict affected flows immediately; revoke credentials; preserve evidence; senior/legal escalation |
| SEV-2 | Primary API/database unavailable, widespread server exploit | Degrade noncritical features; restore service; investigate |
| SEV-3 | Single game abuse, isolated upload or server failure | Quarantine/revoke affected game/server; process report |
| SEV-4 | Noncritical UI or analytics bug | Track and fix through normal release |

## 7. Incident actions

- Stop exposure first (disable package, server, upload route or credential as applicable).
- Preserve minimum necessary audit/evidence records with access controls.
- Identify affected users/data and legal notification requirements with counsel.
- Rotate secrets and invalidate sessions where necessary.
- Rebuild from trusted source; do not trust compromised hosts.
- Communicate status accurately.
- Conduct post-incident review and add regression tests.

## 8. Provider outage behavior

- Worker unavailable: client may use a separately protected origin only if it repeats all auth/authorization checks; otherwise fail closed for sensitive writes.
- PostgreSQL unavailable: block transactions/ownership writes; do not accept Pokoins changes.
- Valkey unavailable: fall back to safe reduced functionality; no loss of durable state.
- R2 unavailable: pause installs/uploads and preserve local installed games.
- Rilavek unavailable/quota exhausted: use authorized direct-R2 fallback or pause upload.
- Hugging Face Space sleeps/restarts: treat backend/authority as unavailable until health/readiness pass; never use Space disk as durable state.
- Community server down: mark session unavailable; no false claim of failover unless state restoration is implemented.

## 9. Acceptance drills

- Restore PostgreSQL into isolated staging.
- Reconstruct a release manifest from object inventory.
- Rotate a server key and revoke the old key.
- Disable a compromised game release and prevent new join tickets.
- Simulate Rilavek outage and direct-R2 fallback.
- Simulate Worker quota/error and confirm no auth bypass.
- Simulate Hugging Face sleep/restart and verify stateless recovery.
