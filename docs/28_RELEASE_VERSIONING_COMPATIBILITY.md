# 28 — Versioning, Compatibility, and Migration Policy

**Status:** Proposed baseline.

## 1. Independent version domains

PokoX must version these separately:

- Poko application version (Client, Studio, Mod).
- Engine ABI/API version.
- Engine runtime/package compatibility version.
- MUTE language version.
- MUTE standard library API version.
- MUTE bytecode version.
- `.pk` project container version.
- `.pkx` player package version.
- `.pkB` authority package version.
- Bitstream protocol major/minor and each schema version.
- Creator Server SDK version.
- Poko Services API version.
- DataStore schema version.

Do not use one global version number to represent all of these compatibility surfaces.

## 2. Semantic compatibility rules

- Major protocol/API version: breaking change.
- Minor version: backward-compatible additions under documented rules.
- Patch version: fixes with no intended contract change.

For package schemas, unknown optional fields may be skipped if format explicitly permits it; unknown required fields must stop loading with a clear error. Never reinterpret a field ID with a new meaning.

## 3. Compatibility manifest

Every release declares minimum and maximum supported engine runtime (or compatible range), MUTE language/API/bytecode version, protocol schema versions, asset format versions and target platforms. Player checks compatibility before installation/launch. Server handshake rejects incompatible protocol/runtime combination.

## 4. Migration strategy

- Project `.pk`: provide Studio migration that creates a backup before modifying.
- Player `.pkx`: package remains immutable; publish a new release.
- Server `.pkB`: pin server package and required runtime compatibility.
- DataStore: versioned schema with explicit migrator; migrations are idempotent and backed up.
- Network schema: support a bounded transition window if feasible; otherwise require coordinated client/server update.
- Creator SDK: publish migration guide and deprecation period before removing APIs.

## 5. Rollback

Rollback means selecting a prior immutable release, not rewriting history. Verify current policy/approval and security revocation. DataStore migrations may be forward-only; if reverse migration is unsafe, document recovery path and restore from backup rather than pretending rollback is automatic.

## 6. Deprecation

Mark deprecated APIs with target removal version and migration instructions. Keep telemetry on use without collecting sensitive content. Do not remove an API merely because a newer engine version exists if supported released games still depend on it; communicate compatibility timelines.

## 7. Acceptance tests

- Client/server mismatch returns a useful compatibility error.
- Unknown required package schema is rejected.
- Project migration creates recoverable backup.
- DataStore migration is idempotent.
- Old approved release cannot bypass current revocation/policy checks.
- Rollback restores exact immutable package hashes.
