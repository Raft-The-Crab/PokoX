# Game Authority and Networking

## Role
Rust service on Huggingface CPU validates consequential game requests and session consistency. It does not continuously simulate every client frame. Runs on dedicated compute with predictable performance.

## Cloudflare Workers for Studio/basic APIs
- Used for Poko Studio and basic platform APIs (asset metadata, discovery feeds, etc.)
- 100k requests/day limit on free tier
- Automatic fallback to Huggingface CPU if limit exceeded
- Provides edge caching and low latency for Studio workflows

## Protocol
- [ ] Packet envelope, message IDs, protocol version, session ID, sequence numbers, size caps and errors.
- [ ] Authentication/session binding.
- [ ] Bounded binary reader/writer with overflow checks.
- [ ] Safe handling of unknown messages.
- [ ] Duplicate/replay policy where needed.
- [ ] Timeouts, heartbeat, disconnect/reconnect.
- [ ] Fuzz parser and malformed payload tests.

## Authority model
- [ ] Classify state as client-predicted, session-authoritative, or backend-authoritative.
- [ ] Validate ownership, range, cooldown, inventory, currency, permissions and game invariants.
- [ ] Never trust client-reported purchases/rewards/privileged actions.
- [ ] Keep account/economy authority in backend.
- [ ] Log validation outcomes without leaking sensitive data.

## Replication
- [ ] Snapshot/delta and interest policy.
- [ ] Reliable/unreliable message rules.
- [ ] Prediction/interpolation/reconciliation only where needed.
- [ ] Bounded queues and backpressure.
- [ ] Tests for loss, duplication, reordering, latency and reconnect.

## Gate
Two clients join and exchange supported state; reconnect works; malicious requests are rejected; malformed packets do not crash the service.
