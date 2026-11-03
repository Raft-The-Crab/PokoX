# 02 — System Architecture and Trust Boundaries

**Status:** Proposed baseline.  
**Architecture style:** Shared native engine + Go modular monolith + edge gateway + independent game-authority runtime.

## 1. Logical architecture

```text
PokoX Client       Poko Studio       Poko Mod
     |                  |                |
     +------------------+----------------+
                        |
                  Poko C++ Engine
          (shared libraries, app-specific hosts)
                        |
       Cloudflare Worker API Gateway / CDN routing
                        |
                Go Platform Backend
         +--------------+-------------------+
         |              |                   |
      Firebase       PostgreSQL          R2 storage
      Identity       source of truth     packages/assets
         |              |
         +----------- Valkey (optional cache)
                        |
                Game Session Registry
                        |
           +------------+------------+
           |                         |
     Creator-hosted             PokoX-hosted
     untrusted operator         platform controlled
           |                         |
           +------------+------------+
                        |
                 Player clients
```

D1 is optional for edge-friendly public metadata/read models, not a second authoritative write database. Cloudinary is for promotional thumbnails/preview transformation, not the main game asset store.

## 2. Trust domains

### Platform domain

Controls identity mapping, platform authorization, marketplace ownership, Pokoins ledger, content approval, package signatures, global reports, sanctions, and platform service quotas.

### Creator server domain

Controls its own host OS, server process, game rules, and game-specific state. PokoX treats operator-supplied claims as untrusted unless independently verified. A server registration proves credential possession, not honest binary execution.

### Client domain

Controls rendering, input, prediction, local cache, and local saves. Client data is inspectable and modifiable. Client-reported achievements and state are untrusted.

### Moderator domain

Contains trusted Poko Mod credentials and moderation controls. It must not load creator-controlled code in the same privileged process. The Review Runtime is a separate restricted process/environment.

## 3. Data ownership

| Data | Authoritative owner |
|---|---|
| Account identity | Firebase identity + PokoX account mapping |
| Username/profile settings | PokoX backend |
| Pokoins balance/ledger | PokoX backend only |
| Platform marketplace ownership | PokoX backend |
| Approved package record | PokoX publishing service |
| Game-specific state | Game server/DataStore namespace, subject to platform limits |
| Client prediction | Client, non-authoritative |
| Offline save | Player device, untrusted until verified |
| Moderation decision | PokoX moderation service |
| Creator server configuration | Creator, bounded by platform policy |

## 4. Request paths

- Account and platform APIs: client → Worker → Go backend → authoritative store.
- Asset download: client → signed/limited download URL → R2/CDN; no need to route every byte through Go.
- Real-time gameplay: client → registered game endpoint using agreed transport; PokoX APIs remain separate.
- Reports: client → PokoX reporting endpoint, never only through the game server.
- Creator server API: registered server → scoped Poko Services API; every operation authorizes game, server, object, and capability.

## 5. Failure domains

A failed Worker must not imply that authorization is skipped. Security-sensitive writes should fail closed. Cached public metadata may remain available. If PostgreSQL is unavailable, financial/ownership writes must stop rather than fall back to an eventually consistent cache. If Valkey is down, noncritical cache features may degrade; critical authority must remain correct without cache. If a free server host sleeps, session availability is lost but persistent data must remain external.

## 6. Modular monolith boundaries

Go backend modules: identity, account/profile, game catalog, publishing, asset metadata, package authorization, server registry, session ticket, DataStore, marketplace, economy, moderation, reporting, notifications, analytics, admin/audit. Modules call internal interfaces and share one controlled database layer. No microservices until measurements or organizational boundaries justify them.

## 7. Interface rules

- Public API contracts are versioned.
- Internal module calls use typed interfaces.
- C++ engine modules do not depend on app-specific UI.
- Poko Mod review process communicates over a narrow authenticated local IPC contract.
- Creator server SDK cannot call internal platform database interfaces.
- All IDs are opaque, non-sequential where exposure could enable enumeration, and scoped appropriately.

## 8. Key architecture invariant

No request from a player or creator process can directly mutate a platform-owned resource without passing through a PokoX service that authenticates, authorizes, validates, rate-limits, and records the operation.
