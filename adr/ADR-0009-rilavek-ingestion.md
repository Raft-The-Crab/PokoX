# ADR-0009 — Rilavek Is an Optional R2 Ingestion Bridge

**Status:** Accepted optional integration  
**Date:** 2026-11-03

## Context
Large creator uploads need resumability, but R2 remains the storage provider and quarantine authority.

## Decision
Direct signed R2 upload is baseline. Rilavek may be used for selected TUS/resumable ingestion, streaming into private R2 quarantine. Backend holds private Sender API token, issues temporary client token, verifies HMAC-SHA256 webhook and processes file IDs idempotently. The current free plan advertises 10 GB monthly transfer and 7-day logs; it is not storage capacity.

## Consequences
- Rilavek outage/limit has direct-R2 fallback.
- Rilavek data terms and privacy must be reviewed before public UGC use.
- Moderation still happens after the object lands in quarantine.
