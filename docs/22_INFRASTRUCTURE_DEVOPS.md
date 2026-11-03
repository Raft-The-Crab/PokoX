# 22 — Infrastructure, Free-Tier Constraints, and DevOps

**Status:** Prototype/closed-alpha infrastructure proposal  
**Budget target:** $0, conditional on current provider free allowances and terms.  
**Review date for provider notes:** 2026-11-03.

## 1. Reality of a $0 infrastructure target

The listed services can support development, demonstrations and a carefully limited alpha. They cannot honestly be treated as a guaranteed always-on public production platform. Free plans can change, sleep, have hard quotas, restrict connections, or remove resources. PokoX must be designed to fail gracefully and remain portable.

Never create multiple accounts to evade provider quotas or terms. Treat each provider's current policy as binding and re-check before deployment.

## 2. Proposed provider allocation

### Cloudflare Workers

Use for edge routing, token/request prechecks, basic throttling, signed upload/download initiation, and cached public reads. Keep Go backend authorization authoritative. The currently documented Free plan lists 100,000 requests/day, 10 ms CPU time, 128 MB memory and 50 subrequests/invocation. Exceeding the daily request quota returns Error 1027. The route's fail-open option bypasses the Worker; do not use bypass as a security fallback for sensitive APIs. Design critical writes to fail closed or route to a separately protected origin that performs the same checks.

### Cloudflare D1

Optional for small edge metadata/read models or lightweight configuration. Current Free docs list up to 10 databases, 500 MB per database and 5 GB account-wide, plus a 50 query/read-subrequest limit per Worker invocation. Do not split authoritative financial state between PostgreSQL and D1. Keep one write owner.

### Cloudflare R2

Primary package/chunk/asset storage. Current Free allowance is 10 GB-month of Standard storage per account, 1 million Class A operations and 10 million Class B operations per month, with internet egress free. It is not 10 GB per bucket. Use content addressing, deduplication, quota management, quarantine and lifecycle cleanup. Monitor operations as well as storage.

### Firebase Authentication

Authentication provider only. Go backend maps Firebase identity to PokoX account and performs its own authorization. Current Firebase Auth quotas include limits on account creation and email operations; check the exact authentication product and project plan at launch. Do not assume SMS verification is free.

### Aiven PostgreSQL

Potential small primary relational store for prototype/alpha. Current free PostgreSQL documentation lists one CPU, 1 GB RAM, 1 GB disk, `max_connections` 20, no connection pooling, no 99.99% SLA, and possible power-off after inactivity. Use a bounded Go pool, migrations, backups and a tested restore. Do not treat it as production HA.

### Aiven Valkey

Optional ephemeral cache. Current free documentation lists 1 CPU, 1 GB RAM and `maxmemory` at 50%, plus possible inactivity power-off. Do not store sole copies of economy or ownership data here.

### Layerbase

Possible development/staging database. Current pricing page advertises two free databases, 5 GB storage and 20 concurrent connections, with sleep/wake behavior. Verify terms and exact engine options. Do not run it as an independently writable production peer to PostgreSQL.

### Cloudinary

Use for promotional imagery and transformations only. Current free plan is 25 monthly credits, not 25 GB. Credit use is shared across transformations, storage and bandwidth. Keep game assets/packages in R2.

### Hugging Face Spaces — two-account topology, conditional eligibility

The intended separation is retained:

- **Hugging Face Account A:** Go platform backend (stateless API/control plane).
- **Hugging Face Account B:** C++ game-authority runtime (stateless/ephemeral session worker where technically compatible).

Each account is a separate failure and credential boundary. Do not share personal access tokens, deployment secrets, or database credentials between accounts. Account separation is for least privilege and operational isolation, not a method to bypass provider quotas or plan restrictions.

**Current eligibility caveat (verified 2026-11-03):** Hugging Face's official Spaces documentation lists CPU Basic as 2 vCPU, 16 GB RAM and 50 GB disk, with no hourly hardware charge. However, it also states that compute-backed Gradio and Docker Spaces require PRO for personal accounts or Team/Enterprise for organizations. Free personal accounts have an exception for up to two Gradio Spaces on ZeroGPU; that is not a general-purpose Go/C++ Docker hosting entitlement. Therefore two ordinary free personal accounts do **not** create two free Docker compute deployments. Use the two-account design only if both accounts independently have the required compute eligibility (paid plan, organization entitlement, grant, or a future documented exception).

Additional limitations:

- Free Spaces sleep after inactivity.
- The documented 50 GB Space disk is not persistent; do not store authoritative records or player data there.
- Spaces networking is designed around web application ingress; test the exact transport before planning real-time game traffic. Do not assume raw UDP or arbitrary inbound TCP support. A WebSocket-over-HTTPS authority prototype may be evaluated, but it is not accepted until latency, lifecycle and reconnect tests pass.
- Keep PostgreSQL/R2 as external persistent systems. The service must tolerate restarts and sleep.
- A sleeping Space is not suitable for guaranteed always-on API or multiplayer hosting.

**Deployment gate:** create and deploy a minimal Go Docker endpoint and a minimal C++ authority endpoint separately, document each account's plan/entitlement, test external reachability and transport, test sleep/restart, and pass credential isolation checks. If either account lacks Docker compute eligibility, run locally for development and use community-hosted servers for multiplayer tests. Do not enable public production service until an eligible, reliable compute path is secured.

Official documentation: https://huggingface.co/docs/hub/spaces-overview

### Rilavek — optional resumable file-ingestion bridge

Rilavek is a managed transfer service, not an object-storage provider. It accepts supported ingress (including HTTP/TUS and SFTP) and streams files into an S3-compatible destination such as Cloudflare R2. Its current Free page advertises 10 GB monthly transfer, seven days of transfer logs and webhook alerts. Files are written to the configured destination; Rilavek does not add storage capacity to R2.

Approved use in PokoX:

- Optional large creator project/backup uploads or controlled bulk-ingestion workflows.
- TUS resumable upload client for unreliable networks.
- Server-generated temporary upload token; the private Sender API token remains only in the Go backend secret store.
- Dedicated R2 quarantine destination/prefix and narrowly scoped destination credentials.
- Verify `X-Rilavek-Signature` using HMAC-SHA256 over the raw webhook body; make `file_id` processing idempotent; confirm the expected object exists before creating a submission record.
- Quarantine all unreviewed content. Rilavek transfer success is not moderation approval.

Do not use Rilavek as the primary package CDN, persistent store, game transport, database, or public approved-content delivery layer. Direct signed R2 uploads remain the fallback and may be preferable for the first implementation. Before public UGC use, review current Rilavek Terms, Privacy, data-processing, credential, webhook-retry and retention behavior. Do not route sensitive incident evidence through a third party without a documented legal/privacy assessment.

Official information: https://rilavek.com/ and https://rilavek.com/docs/sources/http
## 3. Service architecture under free limits

- Worker handles small requests; never proxy package bytes unnecessarily.
- Signed, scoped R2 upload/download flows reduce backend bandwidth.
- PostgreSQL is the only source of truth for transactional platform records.
- D1 is optional read model only.
- Valkey is disposable cache.
- Hugging Face Account A and Account B are separate conditional deployments only when each has independent Docker compute eligibility; otherwise run the services locally or on a separately approved host. All state remains external.
- Backups are stored independently from the primary database/provider where permitted.

## 4. Capacity budgets

Create internal budget counters for daily Worker requests, R2 storage and operations, database size/connections, DataStore per-game use, package size, moderation queue and server slots. Alert at 50%, 70%, 85% and 95% of quota (proposed thresholds). The exact thresholds are configurable. At high usage: throttle optional analytics, pause new large submissions, preserve transactional APIs, and give clear user messages. Never silently drop Pokoins transactions or reports.

## 5. Availability policy

Publicly state that PokoX is in alpha and hosted capacity is limited. Community-hosted servers are best-effort. PokoX-hosted sessions are offered only when slots are allocated. Offline-ready games can remain playable from local packages, but cloud sync, online services and moderation status refresh require connectivity.

## 6. Deployment environments

- Local development.
- Shared development/staging (non-production data).
- Closed alpha.
- Production (only after funded/reliable infrastructure plan).

Never mix test and production signing keys, database credentials, Firebase projects or moderator credentials.

## 7. Secrets

Use provider secret storage or environment secret injection. Never commit secrets to Git, package manifests, client binaries or documentation. Rotate immediately after suspected exposure. Use separate keys per environment and per server instance.

## 8. CI/CD

GitHub Actions or another free CI option can build/test when within its current quota. Pin compilers and dependencies. Build release artifacts reproducibly where possible. Generate checksums/SBOM, run tests and secret/dependency scans. Signing happens in a protected release job, never on an untrusted pull request runner.

## 9. Backups and recovery

Schedule PostgreSQL logical backups, encrypt them, keep more than one recovery point, and test restoration. R2 objects need inventory/manifests and, for critical objects, independent copies or reproducible source backups. Keep source projects in Git/backup snapshots, but do not treat GitHub as a production database backup. Test recovery of identity mappings, economy ledger, marketplace ownership, moderation records and approved release manifests.

## 10. Monitoring

Monitor uptime/health, request rates, Worker quota, DB storage/connection count, R2 storage/operation use, server heartbeats, error rates, review backlog, credential anomalies, backup age and restore results. Alerts should be low-cost and actionable.

## 12. References (verified 2026-11-03; plans may change)

- [Cloudflare Workers limits](https://developers.cloudflare.com/workers/platform/limits/)
- [Cloudflare D1 limits](https://developers.cloudflare.com/d1/platform/limits/)
- [Cloudflare R2 pricing](https://developers.cloudflare.com/r2/pricing/)
- [Firebase Authentication limits](https://firebase.google.com/docs/auth/limits)
- [Aiven PostgreSQL free tier](https://aiven.io/docs/products/postgresql/concepts/pg-free-tier)
- [Aiven Valkey free tier](https://aiven.io/docs/products/valkey/concepts/valkey-free-tier)
- [Layerbase pricing](https://layerbase.com/pricing)
- [Cloudinary billing and plans](https://cloudinary.com/documentation/billing_and_plans)
- [Hugging Face Spaces overview](https://huggingface.co/docs/hub/spaces-overview) — current docs say Docker/Gradio compute needs a paid plan for ordinary personal accounts; free CPU Basic hardware listing does not make free Docker hosting generally available.


## 13. Final provider decision summary

| Provider | PokoX role | Final status |
|---|---|---|
| Cloudflare Workers | Lightweight edge gateway | Selected; strict daily request/CPU budget |
| Cloudflare D1 | Optional edge read model | Optional; no second primary write database |
| Cloudflare R2 | Packages and asset objects | Selected; 10 GB-month Standard free allowance per account |
| Rilavek | Optional TUS/resumable ingest bridge to R2 | Conditional; separate 10 GB monthly transfer allowance, no storage |
| Firebase Auth | Identity authentication | Selected; Go owns PokoX authorization |
| Aiven PostgreSQL | Candidate primary database | Conditional on current free-service eligibility and sleep/connection behavior |
| Aiven Valkey | Ephemeral cache | Optional; never authoritative |
| Layerbase | Staging/secondary evaluation | Optional; never dual-write with PostgreSQL |
| Cloudinary | Promo imagery transformations | Optional; 25 credits/month, not 25 GB |
| Hugging Face Account A | Go API | Conditional Docker compute eligibility |
| Hugging Face Account B | C++ authority | Conditional Docker compute and transport eligibility |

Provider quotas are separate meters. Rilavek's transfer allowance does not increase R2 storage; Cloudinary credits do not increase R2; multiple buckets do not multiply the R2 account storage allowance. Budget and monitor each resource independently.
