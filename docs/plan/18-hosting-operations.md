# Hosting and Operations

## Starting assumptions
APAC-first. 700 CCU is initial target with measured capacity validation. Optimize CPU/RAM based on measured workload. Keep developer-hosted game servers distinct from platform backend workloads.

## Workload placement
- Rust backend (Huggingface CPU): durable platform APIs/workflows.
- Rust game handler/authority (Huggingface CPU): game session coordination and request validation.
- Cloudflare Workers + Rust WASM: Studio/basic APIs (100k req/day, fallback to Huggingface).
- PostgreSQL (Aiven 3GB + Layerbase 5GB): source of truth.
- Aiven Valkey (3GB): cache/session/rate limits.
- Cloudflare R2: assets/packages.
- Cloudflare Workers + D1: targeted edge tasks and edge-local SQLite.
- Firebase Spark Plan: additional services as needed.

## Operations checklist
- [ ] Structured logs with correlation IDs and redaction.
- [ ] Metrics for rate/errors/latency/DB connections/cache/CPU/RAM/sessions/rejections.
- [ ] Actionable alerts and runbooks.
- [ ] Backups and restore drills.
- [ ] Health/readiness, graceful shutdown and draining.
- [ ] Rate limits, backpressure, deployment rollback.
- [ ] Cost/quota alerts and incident response roles.
- [ ] Define availability/data-loss objectives before production.

## Load tests
Idle clients; active APIs; Studio publishing bursts; chat; realistic game sessions; asset downloads; reconnect storms; abusive payloads; DB/cache outage recovery.

## Provider quotas
Maintain a dated limit sheet with exact product/plan, account, window, burst behavior, overage, fallback, verification date and source. Previously discussed Cloudflare figures (100k requests/day, 13k GB-s/day, 5M SQL rows read/day, 100k SQL rows written/day) are historical planning assumptions only; verify current account-specific quotas before use.
