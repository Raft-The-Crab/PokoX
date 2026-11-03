# ADR-0003 — Three Trust Domains and Platform-Owned Services

**Status:** Accepted baseline  
**Date:** 2026-11-03

## Context
Players can modify clients; creators control community-hosted servers; platform assets such as Pokoins and accounts must not be compromised by either.

## Decision
Separate platform, creator-server and player-client trust domains. Platform identity, Pokoins, moderation, publishing approvals and marketplace ownership remain platform-owned. Community server credentials prove identity/possession, not honest code execution. Creator services are game-scoped and capability-limited.

## Consequences
- Creator freedom applies to game logic, not platform administration.
- Platform benefits require independent verification.
- Client/server validation and cross-tenant authorization are release gates.
