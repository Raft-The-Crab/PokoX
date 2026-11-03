# Network Protocols

Schema-defined Bitstream protocol and network layer implementations.

## Components

- **bitstream**: Compact, versioned message serialization (not encryption)
- **schemas**: Message type registry, versioning, validation rules

## Protocol Layers

1. **Transport**: Connection, encryption, reliability, congestion, packet delivery (choice OPEN)
2. **Session protocol**: Handshake, identity, server/game/version negotiation, keepalive
3. **Bitstream codec**: Schema-driven encoding/decoding of typed messages
4. **Gameplay semantics**: Inputs, snapshots, events, authority decisions, replication

## Message Classes

- Reliable ordered: join/leave, inventory transactions, state transitions
- Reliable unordered: selected metadata/events
- Unreliable sequenced: movement snapshots, input state
- Request/response: client actions and server decisions
- Bulk content: packages/assets over HTTPS, not realtime packets

## Transport Decision

OPEN: Select transport after prototype comparing Windows/Android libraries, NAT behavior, reliability, latency, server deployment, encryption, and build complexity. Must support encrypted/authenticated sessions and server certificate validation.

## Security

- Bitstream is serialization, not encryption
- Use established secure transport libraries
- Validate schemas, payload lengths, ranges, and rate budgets
- Anti-replay: sequence spaces, request IDs, bounded replay window
