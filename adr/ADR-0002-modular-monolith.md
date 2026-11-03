# ADR-0002 — Go Modular Monolith for Platform Backend

**Status:** Accepted baseline  
**Date:** 2026-11-03

## Context
An indie project with minimal budget cannot efficiently operate many independently deployed microservices.

## Decision
Start with one Go backend deployment organized into identity, profiles, catalog, publishing, assets, servers, Poko Services, marketplace, economy, moderation, reports, notifications and audit modules. Split services only when measured scale or isolation requirements justify it.

## Consequences
- Lower deployment and operational overhead.
- Clear internal module interfaces required.
- One deployment is a shared failure domain; sensitive signing and transaction paths still require careful isolation.
