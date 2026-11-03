# ADR-0004 — Signed Poko Package Container

**Status:** Accepted direction; binary layout is a prototype gate  
**Date:** 2026-11-03

## Context
The platform must distribute editable projects, player packages and backend packages while detecting tampering and keeping server-only code off clients.

## Decision
Use `.pk` for editable projects, `.pkx` for player content and `.pkB` for server-authority content on a shared versioned PPC container. Proposed cryptographic defaults: SHA-256 content hashes, Ed25519 manifest signatures, canonical CBOR metadata and Zstandard compression.

## Consequences
- Approval binds to exact package hashes.
- Platform private signing keys must remain outside clients/creator hosts.
- Encryption is not treated as protection from device owners.
- Format, signature and parser test vectors must be completed before release.
