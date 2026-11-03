# ADR-0006 — Community-Hosted Game Servers as the Primary Scalable Model

**Status:** Accepted baseline  
**Date:** 2026-11-03

## Context
The initial infrastructure budget is $0 and platform-hosted server capacity is limited.

## Decision
Support local development, creator/community-hosted and limited PokoX-hosted server modes. The official headless C++ runtime plus server-side MUTE is the default. Custom native server integration is deferred. Community servers receive scoped service credentials and are untrusted for platform-wide claims.

## Consequences
- Creators supply their own CPU, RAM, bandwidth and uptime for community hosting.
- PokoX cannot guarantee honesty or availability of community hosts.
- PokoX-hosted competitive/high-integrity modes have stronger control but limited capacity.
