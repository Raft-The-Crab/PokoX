# ADR-0007 — Pokoins Are Virtual at Initial Release

**Status:** Accepted baseline  
**Date:** 2026-11-03

## Context
A real-money marketplace introduces payment processing, tax, consumer protection, refunds, fraud and legal obligations beyond the $0 initial scope.

## Decision
Pokoins are virtual platform currency at launch. No cash-out or real-money conversion. Proposed weekly earning cap is 1,500 PK; marketplace distribution is 85% seller / 15% platform. Actual emission multiplier and sink split require economic simulation before activation.

## Consequences
- Pokoins require a transactional ledger and platform-only grant APIs.
- Marketplace launch is gated by ledger tests and abuse controls.
- Real-money features require a separate future ADR and legal review.
