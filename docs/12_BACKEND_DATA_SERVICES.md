# 12 — Platform Backend, Storage, and Poko Services

**Status:** Proposed baseline  
**Main backend language:** Go  
**Architecture:** Modular monolith first.

## 1. Backend responsibilities

The platform backend owns identity mapping, profiles, catalog, game publishing, package metadata, approvals, server registry, session tickets, platform entitlements, marketplace, Pokoins ledger, moderation/reporting, creator services, notifications and audit logs.

Do not split into many microservices initially. Keep code modules isolated behind interfaces in one deployable Go service. Split only when scale, security isolation, or team ownership justifies it.

## 2. Provider allocation

| Provider | Intended role | Avoid using it for |
|---|---|---|
| Cloudflare Workers | Edge gateway, routing, request shaping, cache | Heavy game simulation or authoritative source of truth |
| Cloudflare R2 | Package/chunk/asset object storage | Relational records or transactional ledger |
| Cloudflare D1 | Small edge read models/configuration if justified | Second independently writable primary database |
| Firebase Auth | User authentication | PokoX authorization/business logic |
| PostgreSQL | Primary transactional store | Unbounded analytics/event firehose |
| Valkey | Cache, ephemeral presence/queues/rate counters | Sole source of truth for currency or ownership |
| Layerbase | Staging/development/migration evaluation | Multi-writer production database beside PostgreSQL |
| Cloudinary | Promo images and preview transformations | Main game package delivery |
| Hugging Face Account A/B | Conditional separate Go API and C++ authority deployments if each has Docker compute eligibility and transport passes | Assumed-free Docker hosting or guaranteed always-on service |

## 3. Identity flow

1. Client authenticates with Firebase.
2. Client receives identity token.
3. Worker or Go API validates token using documented provider verification mechanism.
4. Go maps external UID to internal immutable PokoX UserID.
5. Backend applies PokoX account status and role policy.
6. Backend issues PokoX-scoped session/token as needed.

A valid Firebase token is not permission for arbitrary PokoX actions. Authorization is checked at every endpoint and object.

## 4. PostgreSQL ownership

PostgreSQL is the authoritative transactional source for accounts, profiles, game metadata, submissions/reviews, server registrations, marketplace ownership, Pokoins ledger, moderation and audit references. Use migrations, constraints, transactions, indexes and a bounded connection pool. Never expose database credentials to client or creator game scripts.

## 5. Valkey use

Use for ephemeral presence, server discovery cache, matchmaking queues, short-lived rate counters, temporary locks and cache. Set TTL on ephemeral keys. Design correctness so losing Valkey does not lose Pokoins, purchases, moderation history or persistent game saves. If a rate-limit counter is security-critical, choose a consistent authoritative strategy rather than assuming a cache cannot fail.

## 6. DataStore service

Poko DataStore exposes game-scoped persistence. Each key includes tenant/game scope enforced by backend identity, not only caller-supplied GameID. Support get, set, update, version, compare-and-swap/conditional write, quota, size caps, serialization schema and audit trail. Do not allow a client script to write authoritative persistent progression directly.

## 7. Poko Services contracts

All Poko Services APIs must define: authentication method, authorization scope, request/response schema, idempotency behavior, quota, timeout, retry policy, data retention, error taxonomy and audit fields. Server SDK wrappers must not bypass backend authorization.

## 8. Worker gateway and quotas

Workers can validate basic request shape, apply edge rate limits, route requests and issue controlled upload/download flows. Go must independently validate authentication and authorization. Do not rely on the Worker as the only security boundary.

When Worker limits are reached, never silently bypass security checks. Security-sensitive routes should fail closed. Public cached read-only pages may degrade to cached content. Keep a documented origin fallback only if it performs the same authorization and abuse controls.

## 9. R2 object organization

Store immutable objects by content hash. Keep package manifests and approval records in PostgreSQL; store binary chunks in R2. Quarantine prefixes must not be public. Approved content delivery uses a controlled signed URL/token or equivalent access policy. Separate public approved assets from private creator backups, review evidence and quarantine objects.

## 10. Database schema domains

Suggested domains: `identity`, `accounts`, `games`, `publishing`, `assets`, `servers`, `game_data`, `marketplace`, `economy`, `moderation`, `reports`, `notifications`, `analytics`, `audit`. Avoid over-fragmenting schemas before actual query patterns are known.

## 11. Transaction invariants

- Pokoins debit/credit/fee is one atomic transaction.
- Marketplace ownership and payment record commit together.
- Server credential revocation is immediately effective at the authorization layer.
- Moderation approval references immutable content hashes.
- DataStore update uses concurrency control where concurrent writers could overwrite important state.
- Idempotency keys prevent duplicate actions on retry.

## 12. Acceptance tests

- Invalid Firebase token rejected.
- Valid token without resource permission rejected.
- Cross-game object access denied.
- Worker unavailable does not bypass Go authorization.
- PostgreSQL downtime blocks transactional writes cleanly.
- Valkey loss does not corrupt authoritative records.
- R2 quarantine objects cannot be publicly fetched.
- Pokoins and ownership changes are atomic and idempotent.

## 13. Final upload integration decision

Direct signed R2 upload is the canonical baseline. Rilavek is an optional TUS/resumable ingress bridge for selected creator bulk uploads and backup workflows. The Go backend retains the Rilavek Sender API token and requests short-lived upload tokens; clients receive only the temporary upload token. Upload destination is a private quarantine bucket/prefix. The webhook signature is verified against the raw body using HMAC-SHA256, and `file_id` is processed idempotently. Transfer completion creates a technical-validation job, not an approval record. Never expose R2 credentials or Rilavek private Sender tokens to clients.

## 14. Hugging Face deployment topology

The planned two-account separation is Account A for the Go platform API and Account B for the C++ authority. This is conditional: official Spaces documentation currently requires PRO for personal Docker Spaces or Team/Enterprise for organizational compute Spaces. Two free personal accounts do not provide two free Docker deployments. Each Space can sleep on free hardware and has non-persistent disk; use external PostgreSQL/R2 and stateless process design. The authority transport must be tested; do not assume arbitrary UDP/TCP ingress. See `30_HUGGING_FACE_TWO_ACCOUNT_DEPLOYMENT.md`.

## 15. DataStore consistency baseline

For each game data key, retain `game_id`, namespace, key hash/key identifier, schema version, revision, payload size, updated timestamp and optional ETag/version token. DataStore operations must be authorized by a server credential scoped to the game. Implement `Get`, `Set`, `Update` and conditional update semantics; do not implement unrestricted client writes. Set per-game key count, per-value size, total storage and operation budgets only after load tests against the selected database. A write that times out must be retriable with an idempotency key or compare-and-swap token. Do not silently convert a failed transactional save into success.
