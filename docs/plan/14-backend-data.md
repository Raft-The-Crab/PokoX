# Backend and Data

## Shape
Rust backend on Huggingface CPU (2 VCPUs, 16GB RAM). Two instances: one for main backend, one for game handler. Split services only for measured scaling, reliability, security, or ownership reasons.

## Modules
Identity/session; profiles/preferences; experiences/publishing; asset metadata; discovery/search; favorites/history; economy/ledger/entitlements; moderation/appeals; notifications; developer integrations; admin/audit; health/config.

## Data rules
- PostgreSQL (Aiven 3GB + Layerbase 5GB) is durable source of truth.
- Aiven Valkey (3GB) is cache/session/rate-limit/transient state.
- Cloudflare R2 holds asset/package bytes.
- Cloudflare D1 (SQLite) for edge-local data.
- Currency/inventory mutations are transactional and idempotent.
- Schema changes require migrations, compatibility notes, backup/restore tests and rollback planning.
- No secrets in client bundles or plain logs.

## API tasks
- [ ] Versioning and standard error model.
- [ ] Auth/session and object-level authorization.
- [ ] Request size, pagination, timeout and rate limits.
- [ ] Idempotency keys for retried mutations.
- [ ] Audit security-sensitive operations.
- [ ] Typed contracts and integration tests.
- [ ] Disposable DB tests and backup restore drill.

## Hosting provider commitments
- **Huggingface CPU**: 2 instances × (2 VCPUs, 16GB RAM) for Rust backend and game handler/authority
- **Aiven PostgreSQL**: 3 accounts × 1GB = 3GB total for durable database
- **Aiven Valkey**: 3 accounts × 1GB = 3GB total for cache/session
- **Layerbase**: 5GB usage-based (PostgreSQL or Redis split as needed)
- **Cloudflare R2**: Object storage for assets/packages
- **Cloudflare Workers + D1**: Edge compute for Studio/basic APIs (100k req/day limit, fallback to Huggingface)
- **Firebase Spark Plan**: Additional services as needed

Verify persistence, sleep, ports, networking, usage policy and production guarantees before assigning critical services.
