# 09 — Game Authority and Hosting Model

**Status:** Proposed baseline  
**Hosting modes:** Local development, community-hosted, PokoX-hosted.

## 1. Core model

Games are downloaded and rendered locally. Online multiplayer still needs authoritative session state and replication for dynamic information. PokoX does not need to host every visual world or every game, but a multiplayer session must have an authority that decides valid state transitions.

The client sends input/requests, not trusted final outcomes. The authority validates against game rules, server-owned state, collision/query results and rate limits. Valid results are replicated to relevant clients.

## 2. Hosting modes

### Local development

- Runs on creator's device.
- Private test session by default.
- Synthetic identities/test data.
- No trusted platform rewards.
- Does not appear in public discovery.

### Community-hosted

- Creator operates OS, process, network and capacity.
- PokoX registers identity and scopes service permissions.
- Operator can modify their own host process; registration is not proof of honest execution.
- Server-originated achievements, analytics and game-state claims are untrusted for platform-level benefits.
- PokoX can revoke credentials, stop join-ticket issuance and disable platform services.

### PokoX-hosted

- PokoX controls host environment and official runtime deployment.
- Stronger process isolation, monitoring and package pinning are possible.
- Still runs creator gameplay code inside the MUTE sandbox.
- Capacity is limited and subject to actual resources; no unlimited free hosting promise.

## 3. Authority responsibilities

- Own online player state for the game session.
- Validate movement/action inputs.
- Validate interaction range, cooldowns and game-specific preconditions.
- Manage game inventory and game currency.
- Run server MUTE scripts.
- Emit authoritative events/snapshots.
- Maintain session membership and server heartbeat.
- Call Poko services through scoped APIs.

It does NOT own platform account credentials, Pokoins ledger, global moderation, platform marketplace ownership, or another game's data.

## 4. Server lifecycle

`Created → Registered → Starting → Ready → AcceptingPlayers → Draining → Stopping → Stopped` with failure states. Heartbeat timeout marks a server `Suspect` then `Unavailable`; do not assume a host is dead after one missed heartbeat. Reconnection must use credential and session revalidation.

## 5. Session discovery

1. Player requests a game session.
2. Backend checks game visibility, approval status, client/runtime compatibility and access policy.
3. Registry selects an available server according to region/capacity/party needs.
4. Backend issues a short-lived, one-session join ticket scoped to player, game, server and expiry.
5. Client connects to server.
6. Server validates ticket with platform or an independently verifiable signed ticket format.
7. Server confirms membership; ticket is consumed or replay-protected.

Do not expose arbitrary unverified host endpoints as join destinations.

## 6. Creator server resource model

Creator controls local CPU/RAM; PokoX controls only its own API quotas, storage quotas, service calls and traffic limits. Publish suggested server profiles (small/medium/large) but label them guidance, not enforceable guarantees on third-party hardware. Require heartbeat, version reporting, registered public key, session capacity, region and supported protocol.

## 7. Hosting capacity manager

PokoX-hosted capacity manager should:

- Track allocatable CPU/RAM/process slots and per-region limits.
- Reserve capacity for health/operations overhead.
- Start only approved compatible package versions.
- Stop empty/idle sessions after configurable grace.
- Queue or decline new hosted sessions when full.
- Provide clear player messages.
- Never claim a slot exists until allocation succeeds.

## 8. Community server trust

A creator can modify their server binary. A PokoX-signed `.pkB` proves the package's origin/integrity before execution, not that a community host is still executing it faithfully. For community hosts, constrain what the client accepts: allowed content manifest, protocol schemas, platform service responses, chat filtering, and platform-owned transactions. Mark server trust as `community-unverified` in internal policy and analytics.

For competitive rankings, platform rewards or high-integrity modes, require PokoX-hosted authority or a separately designed verification model. Do not let creator server self-attestation mint Pokoins.

## 9. Server failover

Do not automatically migrate in-memory game state unless the game explicitly supports it. On host loss, mark session unavailable. Players can reconnect to another server if the game supports durable state restoration. DataStore provides persistence, not seamless process migration.

## 10. Acceptance tests

- Registered server can start, heartbeat, accept join tickets and drain.
- Expired/replayed ticket is rejected.
- Revoked credential cannot renew.
- Community server cannot access another game's data namespace.
- Host downtime does not corrupt platform account/economy data.
- Capacity exhaustion returns a truthful response.
- PokoX-hosted instance runs in restricted process/container and terminates cleanly.
