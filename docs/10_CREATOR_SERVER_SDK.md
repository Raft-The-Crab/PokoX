# 10 — Creator Server SDK and Permission Model

**Status:** Proposed baseline  
**Default path:** Official C++ headless runtime + server-side MUTE.  
**Future path:** Custom server integration with scoped Poko Services adapters.

## 1. Purpose

Creators need extensive control over gameplay while PokoX retains control over platform identity, account security, Pokoins, marketplace ownership, moderation, package approval and platform-wide data. The SDK is not a trusted-admin SDK.

## 2. SDK modules

- `PokoServer`: lifecycle, registration, heartbeat, configuration.
- `PokoPlayers`: game-scoped player handles and session management.
- `PokoWorld`: authority-side world/object state.
- `PokoPhysics`: collision queries and simulation integration.
- `PokoNetwork`: schemas, RPC declarations, replication/event helpers.
- `PokoDataStore`: persistent game-scoped storage.
- `PokoMemoryStore`: ephemeral queues/counters/locks with TTL.
- `PokoMessaging`: cross-server game-scoped messages.
- `PokoMatchmaking`: game session search/creation within quota.
- `PokoTeleport`: approved session transfer.
- `PokoTeams`: game team operations.
- `PokoBadges`: game-scoped achievement records.
- `PokoChat`: platform-filtered communications APIs where enabled.
- `PokoAnalytics`: aggregated privacy-limited events.
- `PokoPurchases`: request platform-mediated purchases; never direct ledger writes.
- `PokoReports`: submit safety reports directly to platform.

## 3. Permission classes

### Basic, default game-scoped

Read current session player handles, publish game events, access world objects owned by the game, use approved physics queries, and send bounded replication messages.

### Extended, explicit configuration

DataStore read/write, MemoryStore, messaging, matchmaking, teleport, chat, analytics, and purchase request APIs. Each needs per-game and per-server quotas.

### Sensitive, platform-granted

Special integrations, elevated review/testing APIs, or any access involving sensitive player data. These require separate PokoX review and narrow purpose. Do not expose account admin, Pokoins grant, moderation admin, platform storage admin or cross-game read permissions to ordinary creator servers.

## 4. Authentication

- Each server instance has a unique server ID and public key registration.
- Private key is generated and retained by the operator; never stored in `.pkx`, `.pkB`, source control, or client application.
- Short-lived credentials are issued after proof of possession.
- Credentials bind game ID, server ID, permissions, expiry and key ID.
- Rotate credentials and support immediate revocation.
- Never use one shared API secret across multiple servers.
- Log credential issuance, rotation and revocation without logging secret values.

A credential proves registered identity/possession, not honesty of code execution on a community-controlled host.

## 5. Tenant isolation

Every API operation includes server/game scope derived from authenticated credential, not trusted caller-supplied fields. Verify object ownership and namespace on each operation. IDs are not authorization. A valid game ID in a request does not prove the caller may access it.

## 6. API examples (illustrative)

```lua
local Store = PokoDataStore:GetStore("PlayerProgress")
local progress = Store:GetAsync(player.GameUserId)

PokoTeams:AddPlayer(player, "blue")
PokoReports:Submit({
    category = "safety",
    target = player.GameUserId,
    reason = "..."
})
```

The exact MUTE syntax and result/error conventions remain part of the API specification. DataStore APIs must be asynchronous/budgeted and use version/conditional update semantics for concurrent writes.

## 7. Platform resource quotas

Apply per-game and per-server limits for request count, bytes, storage, object count, message frequency, matchmaking operations, concurrent sessions and analytics volume. Quota exhaustion returns structured errors and does not silently drop transactional writes. Creator local CPU/RAM is not enforceable by PokoX on community hosts; document recommended profiles rather than claiming remote enforcement.

## 8. Purchases and currency

Game-defined Gold/Gems/etc. are game-owned. Pokoins are platform-owned. Creator server can request a purchase flow, but backend checks product ownership, price, balance, eligibility, idempotency and transaction status. No API such as `Pokoins:Give()` exists for creator scripts.

## 9. Reports and communications

Reports must be submitted directly to PokoX. Creator server cannot suppress or delete them. Any chat-enabled SDK must use platform moderation/filters, rate limits and reporting. Do not expose emails, passwords, authentication tokens or player IP addresses to MUTE APIs.

## 10. Custom server integration

Deferred until official runtime is stable. It will use the same scoped service contracts and cannot receive more trust merely because it is written in native C++. Custom implementations must pass protocol conformance, security review, and service authorization tests. They remain untrusted for platform reward verification unless separately validated.

## 11. Acceptance tests

- Cross-game DataStore access denied.
- Expired/revoked credential rejected.
- Credential scope mismatch rejected.
- Purchase retry cannot double-charge.
- Pokoins grant API unavailable.
- Report reaches PokoX independently.
- API quotas enforced server-side.
- Server cannot retrieve player secrets.
- Custom client-crafted API requests receive same authorization checks as SDK calls.

## 12. Final trust interpretation

PokoX does not trust a server merely because it uses the official SDK. A community operator controls the host OS and may modify runtime code. Registration proves possession of a registered key and authorization scope; it does not prove that the expected `.pkB` code is executing honestly. The client and platform must enforce package allowlists, supported protocol schemas, content manifests, permission boundaries, moderation/reporting APIs and platform-mediated transactions independently.

Creator servers may fully control game-specific rules and state within their own game. They may not control platform account identity, Pokoins, marketplace ownership, global reports, moderator powers, other games' namespaces, player authentication secrets or unrestricted personal information. Use game-scoped player pseudonyms where possible. A direct connection exposes the player's network address to the host at the network layer; disclose this clearly. A PokoX relay is a later privacy enhancement, not an initial $0 guarantee.
