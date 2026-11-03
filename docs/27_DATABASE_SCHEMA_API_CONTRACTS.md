# 27 — Core Database Model and API Contracts

**Status:** Logical data model proposal; physical schemas and endpoint paths require ADRs.  
**Primary data store:** PostgreSQL.  
**API style:** Versioned HTTPS control-plane APIs; Bitstream for real-time game traffic.

## 1. Modeling principles

- Use immutable internal UUID/ULID-style identifiers; never use usernames as primary keys.
- Every game-owned table carries a game/tenant scope.
- Every creator-server request derives scope from authenticated credential, not only request body.
- Use UTC timestamps; store user timezone separately for display/weekly task policy.
- Use explicit status enums with legal transition validation.
- Use immutable release/submission/ledger records.
- Use migrations and foreign keys where they improve correctness.
- Keep secrets out of ordinary business tables; store credential hashes/public keys and secret references only.

## 2. Core logical entities

### Identity and profiles

`User`: internal UserID, FirebaseUID (unique), account status, created/updated timestamps.  
`Profile`: UserID, username (unique normalized), display name, avatar asset reference, biography, visibility settings.  
`UserSession`: session ID, UserID, issued/expiry/revocation metadata, device label if disclosed and necessary.  
Never store Firebase passwords or expose raw identity tokens.

### Games and releases

`Game`: GameID, CreatorID, title, description, visibility, content classification, current release ID, moderation state.  
`GameRelease`: ReleaseID, GameID, version, `.pkx` digest, `.pkB` digest, manifest digest, engine/MUTE/protocol requirements, offline mode, submission ID, approval ID, release state and timestamp.  
`GameDependency`: ReleaseID, AssetID, exact asset version/hash, dependency type and permission.

### Assets and libraries

`Asset`: AssetID, CreatorID, asset type, metadata, visibility and license record.  
`AssetVersion`: AssetVersionID, AssetID, version, content hash, object storage key, review status, policy version.  
`Library`: LibraryID, CreatorID, description, current version and listing state.  
`LibraryVersion`: exact package hash, dependency lock, permission manifest, license and review record.

### Moderation

`Submission`: SubmissionID, target type/ID, immutable content hash set, submitter, policy version, status, timestamps.  
`Review`: ReviewID, SubmissionID, moderator ID, reviewer checklist, decision, reason categories and timestamps.  
`Appeal`: AppealID, original decision, appeal text/reference, status, assigned reviewer and result.  
`Report`: ReportID, reporter pseudonymous reference, target, category, evidence references, status and timestamps.

### Servers and sessions

`ServerInstance`: ServerID, GameID, owner, public-key ID, protocol/runtime versions, endpoint metadata, region, status, heartbeat, credential revocation status.  
`GameSession`: SessionID, GameID, ServerID, release ID, capacity, player count snapshot, state and creation time.  
`JoinTicket`: ticket ID, session/player/game scope, expiry, consumed/replay state. Store only what is needed to verify/consume ticket securely.

### Economy and marketplace

`Wallet`: account/currency reference and optional cached balance.  
`LedgerTransaction`: TransactionID, idempotency key, type, status, reference, timestamp.  
`LedgerEntry`: transaction ID, wallet/account, signed amount, currency. Entries are immutable.  
`Listing`: listing ID, owner, exact version, price, payment type, status, license.  
`OwnershipGrant`: buyer, item/version, purchase transaction and license version.  
`RewardClaim`: user, task/cycle, eligibility and transaction reference.

### DataStore

`GameDataKey`: game namespace, key, schema version, value blob/reference, revision, size, updated time. Use per-game authorization and size caps. For larger scale, partition by game or key strategy only after query/performance evidence.

## 3. Important indexes/constraints

- Unique FirebaseUID.
- Unique normalized username.
- Unique idempotency key within operation scope.
- Unique `(GameID, version)` or unique release version policy.
- Unique approved hash/policy mapping where reuse is allowed.
- Foreign key from release to game/creator and submission/approval.
- Foreign key from ledger entries to transaction.
- Check constraints for nonnegative prices and legal enum values.
- Game-scoped composite indexes for DataStore and assets.

## 4. API contract template

Every endpoint document must define:

- Method/path/version.
- Caller identity and required role/capability.
- Object-level authorization rule.
- Request/response schema.
- Maximum request/response size.
- Idempotency/replay behavior.
- Rate/byte quota.
- Timeout/retry guidance.
- Error codes.
- Audit event.
- Data retention and privacy classification.

## 5. Suggested API groups

- `/v1/auth/session` — PokoX session establishment after identity verification.
- `/v1/profile/*` — profile and privacy settings.
- `/v1/catalog/*` — public approved game discovery.
- `/v1/releases/*` — creator submission/status and published release metadata.
- `/v1/assets/*` — signed upload/download initiation and metadata.
- `/v1/servers/*` — server registration, heartbeat, credential refresh/revoke.
- `/v1/sessions/*` — session discovery and join ticket issuance.
- `/v1/datastore/*` — game-scoped persistence.
- `/v1/reports/*` — direct-to-platform reports.
- `/v1/marketplace/*` — listings, purchase requests and ownership queries.
- `/v1/economy/*` — player task/reward status and ledger history subject to privacy.

Paths are illustrative; final routes and HTTP semantics are OPEN.

## 6. Error taxonomy

Use stable machine-readable error codes such as `AUTH_REQUIRED`, `FORBIDDEN`, `NOT_FOUND`, `VERSION_CONFLICT`, `RATE_LIMITED`, `QUOTA_EXCEEDED`, `INVALID_SCHEMA`, `PACKAGE_REJECTED`, `SESSION_UNAVAILABLE`, `SERVICE_UNAVAILABLE`, `IDEMPOTENCY_CONFLICT`. Avoid revealing sensitive existence information through error differences where enumeration is a risk.

## 7. Consistency and transactions

Use PostgreSQL transactions for ownership, Pokoins, moderation decisions and credential revocation state. Use optimistic concurrency or row locking for contested game data writes. Cache invalidation must not be the source of truth. D1 read replicas/read models must be rebuildable from the authoritative backend.

## 8. Retention and deletion

Define retention separately for account records, audit, moderation reports, safety evidence, analytics and game DataStore. Provide deletion/anonymization workflow consistent with legal obligations and dispute/evidence holds. Do not delete financial/audit records simply because a user changes username.

## 9. Acceptance tests

- Cross-game reads/writes denied even with guessed IDs.
- Duplicate idempotency key returns original outcome.
- Concurrent DataStore updates do not silently overwrite when conditional update is required.
- Game release cannot point to an unapproved hash.
- Ledger entry references valid transaction and balances per transaction.
- Revoked server cannot renew credential.
- API error responses do not expose secrets/internal SQL details.
