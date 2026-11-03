# 23 — Testing, QA, Security Verification, and Release Gates

**Status:** Proposed engineering quality baseline.

## 1. Testing pyramid

- Unit tests: math, parsers, serializers, ledger calculation, permission evaluation.
- Component tests: renderer resource lifecycle, physics query, package reader, MUTE runtime.
- Integration tests: Go/PostgreSQL, Worker/backend, R2 uploads, Firebase identity mapping, DataStore.
- End-to-end tests: create project → build → submit → review → publish → download → install → play.
- Fuzz tests: every hostile binary/text input parser.
- Performance tests: render frame, physics, package build/download, Bitstream decode, server tick.
- Security tests: trust boundaries, tenant isolation, credential revocation, abuse and role checks.

## 2. Required test suites

### Engine

- World serialization round-trip.
- Handle generation/stale handle.
- ECS query correctness.
- Renderer context lifecycle and resource cleanup.
- Input mapping.
- Physics collision and finite-value validation.

### MUTE

- Lexer/parser grammar.
- Type checker.
- Realm/API availability.
- Capability denial.
- Execution budgets.
- Memory cap.
- Bytecode verifier (when VM exists).
- Cross-script isolation.

### Packages

- Header/manifest/index fuzzing.
- Path traversal rejection.
- Decompression limits.
- Hash/signature verification.
- Chunk cache correctness.
- Approval invalidation after content change.

### Network and authority

- Schema encode/decode.
- Version negotiation.
- Replay/duplicate handling.
- Out-of-range field rejection.
- Movement/combat validation.
- Snapshot recovery.
- Flood/rate tests.

### Backend/economy

- Cross-tenant authorization.
- Firebase token invalidation.
- Idempotency race tests.
- Atomic 85/15 transaction.
- Refund/reversal consistency.
- DataStore concurrent update and quotas.
- Server credential rotation/revocation.

### Moderation and safety

- No publish without human decision.
- Evidence bound to hash.
- Appeal workflow.
- Report path independent of game server.
- Review Runtime cannot access production secrets.
- Sensitive evidence access audit.

### Offline

- Full offline launch.
- Missing/corrupt asset recovery.
- Local save persistence.
- Cloud sync conflict.
- Offline Pokoins denial.
- Revocation freshness limitation disclosed.

## 3. Fuzzing priorities

1. PPC package parser.
2. MUTE parser and VM bytecode verifier.
3. Bitstream decoder.
4. Game project deserializer.
5. Asset importers (GLB, images, audio).
6. Network/API request decoders.

Fuzz targets must run in CI or scheduled jobs with reproducible seeds and crash corpus retention. Run sanitizers in native debug builds where supported.

## 4. Security verification baseline

Use OWASP Game Security Framework for game-specific requirements and OWASP API Security Top 10 for APIs. Map each applicable control to test cases. Use OWASP MASVS or relevant mobile guidance for Android client hardening where applicable. External testing is required before handling meaningful public accounts/economy.

## 5. Release channels

- Developer local build.
- Internal development build.
- Closed alpha.
- Public beta.
- Stable release.

Each release records engine, MUTE, package, protocol, backend API and SDK versions. Use staged rollout and rollback for application updates. Game publishing still follows human review.

## 6. Definition of done

A module is not done because it compiles. It needs documented public interface, tests for normal/error/hostile inputs, resource cleanup, logging policy, security review for exposed data, performance baseline where relevant, and documentation. Public-facing changes need migration notes.

## 7. Launch gates

### Gate A — playable prototype

Renderer, input, simple world, character physics, MUTE subset and local package work.

### Gate B — multiplayer alpha

Authoritative actions, Bitstream validation, session join, server revocation and basic moderation tools work.

### Gate C — creator alpha

Studio saves/exports, publishing queue, human review and creator server SDK work end-to-end.

### Gate D — public beta

Privacy policy, child safety procedures, report/block, moderation staffing plan, backups, restore test, incident response, quota dashboards, and security review are complete.

### Gate E — economy activation

Pokoins ledger invariants, purchase idempotency, fees, refunds, anti-abuse and audit coverage pass; marketplace moderation is staffed.

## 8. References

- [OWASP Game Security Framework](https://owasp.org/projects/gamesec-framework)
- [OWASP API Security](https://owasp.org/www-project-api-security/)
- [OWASP MASVS](https://mas.owasp.org/MASVS/)
