# 11 — Poko Files and Package Container Specification

**Status:** Proposed format architecture; binary header and canonical manifest schema are OPEN.  
**Extensions:** `.pk` project, `.pkx` player release, `.pkB` game-authority package.

## 1. Package goals

- Immutable, versioned and verifiable releases.
- Content-addressed chunk reuse.
- Separate authoring, client and server content.
- Incremental downloads.
- Safe parsing of untrusted archives.
- Reproducible build metadata where practical.
- Signed approval/release records.

Do not treat an extension as proof of safety. Every package is untrusted until parsed, validated and checked against platform release records.

## 2. Common Poko Package Container (PPC)

Proposed logical sections:

1. Header: magic, container version, package kind, flags and bounded section offsets.
2. Manifest: package ID, owner/publisher, version, engine/MUTE/protocol compatibility, dependencies, platform support, offline declaration and content labels.
3. Index: normalized paths, content IDs, hashes, compressed/uncompressed sizes, offsets, media type and chunk metadata.
4. Chunks: compressed payload blocks.
5. Integrity metadata: hashes, manifest digest, signature metadata and key ID.

The exact binary layout must be written as a separate format document before implementation. It must define byte order, alignment, integer encoding, overflow checks, canonical representation, path encoding, maximum counts/sizes and unknown-field behavior.

## 3. File roles

### `.pk`

Editable project/backup. May contain authoring source, full asset sources, scenes, UI documents, MUTE source, project settings, dependencies and editor metadata. Imported `.pk` content opens in safe mode; scripts are not automatically executed merely by importing a project.

### `.pkx`

Player-distributed client package. Contains public gameplay assets, runtime UI, client scripts and non-secret shared definitions. Must not contain server-only MUTE, private server configuration, credentials, database keys or platform secrets.

### `.pkB`

Server-authority package. Contains server MUTE, game rules, server state definitions, server permissions, protocol schemas and required server dependencies. It is loaded only by an authorized game authority/review process. A creator-hosted operator can inspect their own server package; do not promise secrecy from the operator.

## 4. Cryptography

- SHA-256 content hash for chunk integrity/deduplication.
- Ed25519 or another reviewed signature scheme for release manifests, subject to crypto library review and key-management decision.
- TLS for network transfer.
- Public verification keys in clients; private signing keys only in controlled platform signing infrastructure.
- Key IDs, rotation, revocation, signature algorithm/version fields.
- Never invent encryption/signature algorithms.

Sign the canonical manifest and its complete content-hash list. A signature proves publisher/platform approval of the referenced content, not that client code cannot be reverse-engineered or that a community server is executing the package unchanged.

## 5. Chunking and deduplication

Chunks are addressed by cryptographic hash. Package indexes reference chunk IDs. Download manager checks local cache before fetching. Chunks have bounded compressed/uncompressed sizes. Use a mature compression format such as Zstandard only after library/build/license review. Deduplicate exact content; do not deduplicate across security/permission contexts if doing so leaks private object existence or mixes access control.

## 6. Safe parsing

- Validate header before allocating.
- Bounds-check all offsets and lengths with overflow-safe arithmetic.
- Reject overlapping sections where not allowed.
- Normalize paths and reject traversal/absolute paths/device paths.
- Reject duplicate normalized paths.
- Cap file count, nesting, manifest size, chunk size, total uncompressed size and compression ratio.
- Reject unsupported executable/native payload types.
- Verify hashes before use.
- Never blindly extract to arbitrary filesystem paths; use a virtual asset filesystem or controlled staging directory.
- Parse in a restricted process for moderation and import of untrusted packages.

## 7. Release identity

An approval references the exact package digest, content hashes, game ID, release version, policy version, engine/MUTE/protocol compatibility, review decision and approval timestamp. Changing any signed content produces a new digest and invalidates the old approval.

## 8. Offline cache

Installed chunks stay in a managed cache. Local saves are stored separately. Cache eviction uses reference counts across installed packages and must never remove a chunk still referenced by an installed game. A package's platform signature and locally cached revocation metadata are checked at launch; offline clients cannot receive new revocation information until reconnecting.

## 9. Encryption clarification

Encryption in transit and at rest protects transfer/storage. It does not make client assets secret once decrypted for rendering/execution. Obfuscation and bytecode compilation are friction only. Sensitive authority code must not be shipped in `.pkx`.

## 10. Acceptance tests

- Corrupt header, offset, size and hash rejected.
- Invalid signature rejected.
- Traversal and duplicate normalized paths rejected.
- Decompression bomb bounded.
- Unsupported executable payload rejected.
- Identical chunks deduplicate.
- Updating one asset downloads only missing chunks.
- Client package contains no server-only scripts/secrets.
- Review approval invalidated by content hash change.
