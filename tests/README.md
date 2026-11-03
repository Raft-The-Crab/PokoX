# PokoX Test Suite

Comprehensive testing strategy for the PokoX platform.

## Test Categories

- **unit**: Component-level tests (math, parsers, serializers, ledger, permissions)
- **integration**: Cross-component tests (Go/PostgreSQL, Worker/backend, R2 uploads, Firebase identity)
- **fuzz**: Fuzz testing for hostile binary/text input parsers
- **performance**: Render frame, physics, package build/download, Bitstream decode, server tick
- **security**: Trust boundaries, tenant isolation, credential revocation, abuse, role checks
- **end_to_end**: Full workflow tests (create → build → submit → review → publish → download → play)

## Fuzzing Priorities

1. PPC package parser
2. MUTE parser and VM bytecode verifier
3. Bitstream decoder
4. Game project deserializer
5. Asset importers (GLB, images, audio)
6. Network/API request decoders

## Required Test Suites

### Engine
- World serialization round-trip
- Handle generation/stale handle
- ECS query correctness
- Renderer context lifecycle and resource cleanup
- Input mapping
- Physics collision and finite-value validation

### MUTE
- Lexer/parser grammar
- Type checker
- Realm/API availability
- Capability denial
- Execution budgets
- Memory cap
- Bytecode verifier (when VM exists)
- Cross-script isolation

### Packages
- Header/manifest/index fuzzing
- Path traversal rejection
- Decompression limits
- Hash/signature verification
- Chunk cache correctness
- Approval invalidation after content change

### Network and Authority
- Schema encode/decode
- Version negotiation
- Replay/duplicate handling
- Out-of-range field rejection
- Movement/combat validation
- Snapshot recovery
- Flood/rate tests

### Backend/Economy
- Cross-tenant authorization
- Firebase token invalidation
- Idempotency race tests
- Atomic 85/15 transaction
- Refund/reversal consistency
- DataStore concurrent update and quotas
- Server credential rotation/revocation

### Moderation and Safety
- No publish without human decision
- Evidence bound to hash
- Appeal workflow
- Report path independent of game server
- Review Runtime cannot access production secrets
- Sensitive evidence access audit

### Offline
- Full offline launch
- Missing/corrupt asset recovery
- Local save persistence
- Cloud sync conflict
- Offline Pokoins denial
- Revocation freshness limitation disclosed
