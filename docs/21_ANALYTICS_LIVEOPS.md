# 21 — Analytics, Diagnostics, and Live Operations

**Status:** Proposed baseline  
**Principle:** Collect only what is useful, disclose collection, and keep creator analytics aggregated by default.

## 1. Platform analytics

Measure operational facts: API success/error rates, package downloads, install failures, server availability, review backlog, queue age, moderation response times, storage use, Worker quota, database health, report volume and incident metrics. Avoid collecting unnecessary personal data.

## 2. Creator analytics

- Visits and unique players (define counting rules).
- Concurrent players.
- Session start/finish and crash rate.
- Average session duration where privacy policy permits.
- Favorites and return rate.
- Download/launch failures.
- Client performance aggregates.
- Server tick/heartbeat health.

Creators should not receive player email, IP, authentication identifier, precise location or raw private chat. Use minimum cohort sizes for aggregate reports to reduce re-identification risk.

## 3. Telemetry event schema

Each event includes event ID, event type, timestamp, game/release version, session ID where necessary, coarse platform class, aggregation scope and retention class. Avoid raw free-text fields. Redact secrets and sensitive identifiers. Enforce event size/rate limits at ingestion.

## 4. LiveOps

Features may include scheduled announcements, maintenance windows, configuration flags, event schedules, session drain/restart requests, rollback and compatibility minimums. Live config must be schema-validated, permission-scoped and audited. Content-changing updates (scripts/assets/animations/maps) require publishing review. LiveOps must not become a moderation bypass.

## 5. Operational dashboards

Platform dashboard: service status, error budget, database storage/connections, R2 storage/operations, Worker request usage, queue size, backup status, active sessions and alerts. Poko Mod server view should display only controlled diagnostics and avoid exposing host secrets.

## 6. Privacy and retention

Publish what is collected and why. Define separate retention for operational logs, gameplay aggregates, reports and safety evidence. Restrict access by role and record access. Support deletion/export obligations as legally required. Do not use analytics to infer sensitive traits or target children with manipulative recommendations.

## 7. Incident response

Define severity levels, incident owner, containment actions, credential rotation, player/creator communication, evidence preservation, postmortem and corrective actions. A service incident is not resolved until restoration is verified and affected data checked.

## 8. Acceptance tests

- Creator analytics are aggregated and privacy-scoped.
- Event ingestion rejects oversized/flooded events.
- Live config cannot change unapproved game content.
- Operational alert triggers on quota threshold.
- Audit log records destructive LiveOps action.
- Telemetry redacts credentials and sensitive fields.
