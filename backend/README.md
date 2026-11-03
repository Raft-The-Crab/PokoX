# Poko Platform Backend

Go modular monolith backend for the PokoX platform.

## Architecture

One Go deployment organized into internal modules. Split into microservices only when scale or isolation requirements justify it.

## Modules

- **api**: HTTP/HTTPS API endpoints and routing
- **auth**: Firebase identity mapping and PokoX session management
- **catalog**: Game discovery, search, and public metadata
- **publishing**: Submission workflow, quarantine, and release management
- **assets**: Asset upload, validation, and metadata
- **servers**: Server registration, heartbeat, and credential management
- **marketplace**: Listings, purchases, ownership, and licensing
- **economy**: Pokoins ledger, rewards, and transaction processing
- **moderation**: Review queue, decisions, appeals, and evidence
- **reports**: Player safety reports and case management
- **notifications**: In-app notifications and alerts
- **analytics**: Aggregated analytics and creator metrics
- **audit**: Audit logging and compliance records
- **modules**: Shared internal interfaces and utilities
- **migrations**: Database schema migrations

## Data Stores

- **PostgreSQL**: Primary transactional source of truth
- **Valkey**: Ephemeral cache, queues, rate counters (disposable)
- **Cloudflare R2**: Package chunks, assets, and object storage
- **Cloudflare D1**: Optional edge read models (rebuildable only)

## Provider Architecture

- Firebase Auth: Identity authentication only
- Cloudflare Workers: Edge gateway, request shaping, cache
- All PokoX authorization happens in Go backend
