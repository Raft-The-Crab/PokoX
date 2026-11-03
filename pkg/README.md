# Poko Package Formats

Implementation of the Poko Package Container (PPC) and related utilities.

## Package Types

- **.pk**: Editable project/backup (source projects, full assets, scenes, UI, MUTE source)
- **.pkx**: Player release package (client assets, client scripts, shared definitions)
- **.pkB**: Server-authority package (server MUTE, game rules, server permissions)

## Components

- **package**: PPC container parser, header, manifest, file index, chunk management
- **crypto**: SHA-256 content hashing, Ed25519 signature verification, key management
- **compression**: Zstandard compression/decompression with limits

## Security

- Platform-signed release manifests
- Content-addressed chunk storage
- Hash verification before load
- Safe parsing with bounds checking
- Fuzz testing for all parsers

## Cryptography Defaults (subject to review)

- SHA-256 for content hashes
- Ed25519 for manifest signatures
- Canonical CBOR for metadata
- Zstandard for compression

Note: Exact binary layout, key lifecycle, and rotation procedures require test vectors and independent review before release.
