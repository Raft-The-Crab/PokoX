# Service Limits and Capacity

Provider quotas change; update this file with date and source before production deployment.

## Confirmed provider commitments

**Huggingface CPU**: 2 instances × (2 VCPUs, 16GB RAM) - Continuous
**Aiven PostgreSQL**: 3 accounts × 1GB each = 3GB total - Persistent
**Aiven Valkey**: 3 accounts × 1GB each = 3GB total - Persistent
**Layerbase**: 5GB usage-based (PostgreSQL or Redis split) - Persistent
**Cloudflare R2**: Object storage - TBD
**Cloudflare Workers**: Free tier - 100,000 requests/day limit
**Cloudflare D1**: SQLite - TBD
**Cloudinary**: Free tier - 25 credits/month (~25GB storage + 25GB bandwidth)
**Firebase Spark Plan**: Free tier - TBD

**Cloudflare Workers usage strategy**:
- Used for Poko Studio and basic platform APIs (asset metadata, discovery feeds, etc.)
- 100k req/day limit on free tier
- Automatic fallback to Huggingface CPU when limit exceeded
- Monitor usage and alert before hitting limit

**Cloudinary usage strategy**:
- Used for processed images (textures, thumbnails, icons, screenshots)
- 25 credits/month limit on free tier
- Fallback to R2 for raw images if credits exhausted
- Monitor usage and alert before credit exhaustion

## Capacity targets
- Initial target: 700 CCU with measured validation
- Load test target: 5,000 CCU (not a capacity guarantee)
- Actual capacity depends on measured performance on above infrastructure

## Quota register (add as verified)

**Cloudflare Workers**:
- Requests/day: TBD
- CPU time: TBD
- Memory: TBD

**Cloudflare D1**:
- Storage: TBD
- Read units/day: TBD
- Write units/day: TBD

**Cloudflare R2**:
- Storage: TBD
- Egress: TBD

**Cloudinary**:
- Storage: 25GB (25 credits)
- Bandwidth: 25GB (25 credits)
- Transformations: TBD
- Alerts at: 20 credits (80% usage)

**Aiven PostgreSQL**:
- Storage: 3GB total
- Connections: TBD
- IOPS: TBD

**Aiven Valkey**:
- Storage: 3GB total
- Connections: TBD
- Operations/sec: TBD

**Layerbase**:
- Storage: 5GB total
- Connections: TBD

## Capacity method
1. Define idle, active API, chat, game session, Studio publish, and asset-download workloads.
2. Measure resource use per profile.
3. Measure per-session memory/message rate.
4. Isolate CPU, RAM, DB connections/query cost, cache, network and egress bottlenecks.
5. Include reconnect storms and dependency failures.
6. Estimate costs from measured traffic and actual prices.
7. Run staged load tests and preserve results.

Alert before quotas; bound queues and payloads; define degraded modes; never place critical durable writes on an unbounded/quota-fragile path.
