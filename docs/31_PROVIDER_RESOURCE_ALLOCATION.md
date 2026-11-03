# 31 — Final Provider Resource Allocation and Capacity Budget

**Status:** Selected architecture with conditional provider availability.  
**Budget objective:** $0 where provider eligibility and free limits allow.

## 1. Resource ownership

| Resource | Owner of truth | Provider/technology | Notes |
|---|---|---|---|
| Account identity | PokoX mapping + Firebase | Firebase Auth + Go | Firebase authenticates; Go authorizes |
| Platform records | Go backend | PostgreSQL | One authoritative writer |
| Temporary cache | Backend | Valkey | Disposable; TTL required |
| Edge config/read model | Backend-derived | D1 (optional) | Rebuildable; not second primary |
| Binary packages/assets | Package manifests + R2 objects | Cloudflare R2 | 10 GB-month Standard/account free allowance |
| Large resumable ingress | Transfer events + R2 object | Rilavek optional | 10 GB monthly transfer free; not storage |
| Promo images | Asset metadata + Cloudinary | Cloudinary optional | 25 credits/month, not 25 GB |
| Go API compute | Deployable Go image | HF Account A conditional | Requires eligible Docker compute |
| C++ authority compute | Deployable C++ image | HF Account B conditional | Requires eligible compute and transport compatibility |
| Development/staging DB | Non-production only | Layerbase/Aiven alternatives | No dual-write |

## 2. Cloudflare account math

- R2 Free: 10 GB-month Standard storage per account; 1 million Class A operations/month; 10 million Class B operations/month; free internet egress under current documentation.
- The 10 GB is account-wide, not per bucket. Buckets organize access and lifecycle but do not create more free storage.
- D1 Free: currently documented per-database and account-wide caps; re-check current limits before provisioning. Keep D1 as optional read/configuration data.
- Workers Free: currently documented 100,000 requests/day; monitor request count and CPU budget. Avoid sending game ticks and package payload bytes through Workers.

## 3. Rilavek math

Rilavek Free currently advertises 10 GB monthly transfer and seven days of transfer logs. This is transfer throughput, not persistent storage. If a 2 GB creator package is uploaded through Rilavek, it consumes transfer allowance and the resulting object consumes R2 storage while retained. Deleting an R2 object frees storage over the applicable billing measurement; it does not restore already-consumed Rilavek transfer for that month.

## 4. Quota thresholds

Proposed internal warning thresholds: 50%, 70%, 85%, 95%. At 85% pause nonessential large backups and optional analytics. At 95% pause new optional uploads/large package submissions and notify users. Never silently drop purchases, reports, moderation audit events or critical account writes. These thresholds are PokoX policy, not provider quotas.

## 5. Storage lifecycle

- Quarantine has a short policy-defined retention window, except evidence/legal holds.
- Deduplicate immutable asset chunks by content hash.
- Keep active release chunks and required versions.
- Remove unreferenced cache chunks only after reference-count and grace-period checks.
- Creator source backups have separate quotas and deletion controls.
- Moderation evidence is private and access-logged.
- Do not store user uploads in Cloudinary as a substitute for R2.

## 6. Multi-provider rule

Never use multiple databases as simultaneous independent writers for the same records. Pick one PostgreSQL authority. D1 is read model/config only; Valkey is cache; Layerbase is staging/alternative; R2 is object store. Add a provider adapter only where it preserves consistent semantics and can be tested.

## 7. Free-tier operational policy

- Closed-alpha user counts and submission volumes are capped by measured capacity.
- PokoX-hosted game slots remain a scarce beta feature.
- Community-hosted servers carry most multiplayer capacity.
- Offline-ready games remain locally playable when platform services are unavailable, subject to cached signed approval policy.
- A public release requires a real hosting, legal and moderation capacity plan; $0 is an initial constraint, not an uptime SLA.
