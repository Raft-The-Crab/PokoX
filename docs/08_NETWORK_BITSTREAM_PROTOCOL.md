# 08 — Schema-Defined Bitstream Protocol

**Status:** Architectural baseline; byte-level wire format and transport selection are OPEN.  
**Design goal:** Compact, versioned, validated messages without JSON in the real-time gameplay path.

## 1. Protocol layers

Keep these concerns separate:

1. **Transport:** connection, encryption, reliability, congestion and packet delivery.
2. **Session protocol:** handshake, identity, server/game/version negotiation, keepalive and rejoin.
3. **Bitstream codec:** schema-driven encoding/decoding of typed messages.
4. **Gameplay semantics:** inputs, snapshots, events, authority decisions and replication.

Bitstream is a serialization format, not an encryption mechanism or a transport protocol. Use established secure transport/cryptographic libraries; do not invent packet encryption or key exchange.

## 2. Transport decision

OPEN: select a practical transport after a prototype comparing supported Windows and Android libraries, NAT behavior, reliability, latency, server deployment, encryption, licensing, and MSYS2/Android build complexity. Candidate architecture may use a reliable secure control channel plus a low-latency datagram transport, or a mature unified transport. Do not commit to raw custom UDP reliability until the team can implement and test congestion control, retransmission, packet ordering and abuse protection.

All transport choices must support encrypted/authenticated sessions and server certificate/identity validation. Transport details are recorded in an ADR.

## 3. Message classes

- Reliable ordered: join/leave, inventory transaction result, game state transitions, ownership changes.
- Reliable unordered or independently sequenced: selected metadata/events.
- Unreliable sequenced: frequent movement snapshots and disposable aim/input state.
- Request/response: client actions and server decisions with request IDs.
- Bulk content: package/assets over HTTPS/R2-style delivery, not realtime gameplay packets.

Class semantics must be specified per message, not guessed by caller.

## 4. Schema registry

Every message has a stable numeric type ID, schema version, direction, maximum encoded length, delivery class, rate budget, authorization rule and field constraints. Schema changes are versioned. Additive optional fields may be compatible under explicit rules; renaming/retyping/reusing field IDs is not. Old fields are reserved, not recycled.

## 5. Proposed logical envelope

```text
MessageEnvelope
  protocol_major
  protocol_minor
  message_type
  schema_version
  sequence_number
  request_id (when required)
  payload_bit_length
  payload
```

Exact field widths, alignment, endianness and optional compression are OPEN and must be specified in a binary-format document after prototype tests. Parser must reject truncated, oversized, over-nested, unknown-required, invalid-range, and trailing-garbage messages according to message policy.

## 6. Example input schema (logical)

```text
PlayerInput {
  sequence: u32
  client_tick: u32
  action_flags: bitset
  move_x: bounded signed quantized value
  move_y: bounded signed quantized value
  view_rotation: validated quantized representation
}
```

Client input expresses intent; it is not an authoritative position or reward. The server derives movement from accepted input and game rules. Client tick is a hint for prediction/reconciliation and must not grant arbitrary time advancement.

## 7. Validation

- Authenticate connection/session.
- Verify game ID, server ID, player membership and message direction.
- Validate schema and payload length before allocation.
- Validate sequence/replay policy.
- Validate numeric ranges and finite values.
- Enforce per-message and per-session rate/byte budgets.
- Validate game-specific state transitions.
- Record security-relevant rejects without logging sensitive payloads unnecessarily.

## 8. Replication

Use interest management so each client receives only relevant entities/events. Use delta snapshots against acknowledged baselines, periodic full baseline snapshots, explicit entity generation IDs, and server timestamps/ticks. Do not rely on delta-only state forever; lost baselines need recovery. Use compact entity component masks and versioned component schemas.

## 9. Client prediction and reconciliation

Client predicts input locally. Server processes permitted inputs and produces authoritative state. Client reconciles to authoritative state and replays unacknowledged inputs within bounded history. Avoid assuming floating-point deterministic simulation across different CPUs/GPUs. Critical actions such as purchases, trades, item grants and damage results must be server-authorized.

## 10. Anti-replay

Use per-session sequence spaces, request IDs/idempotency for transactional operations, and a bounded replay window. Do not treat a client-generated nonce by itself as proof that a request is genuine. Transactional endpoints must be idempotent at the service layer.

## 11. Acceptance tests

- Encode/decode round-trip for each registered schema.
- Cross-version compatibility tests.
- Fuzz invalid bit lengths, varints (if used), nested collections and lengths.
- Packet replay rejected or deduplicated according to class.
- Oversized and high-rate packets throttled.
- Snapshot loss recovers through full baseline.
- Client cannot directly set authoritative transform/health/inventory.
- Packet capture contains no plaintext secrets or credentials.
