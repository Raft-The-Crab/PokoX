# 19 — Pokoins Economy and Creator Marketplace

**Status:** Proposed economy design; launch only after transaction controls are proven.  
**Currency:** Pokoins (PK), virtual platform currency.

## 1. Economy principles

- Pokoins are platform-owned virtual currency.
- No direct client or creator-server balance edits.
- No real-money conversion or cash withdrawal in initial release.
- Every economic change has a ledger record.
- Platform rewards are verified by PokoX services.
- Offline play cannot mint or spend Pokoins.
- Marketplace prices and seller terms are clear.

## 2. Weekly earning allowance

Project direction: each eligible user can earn up to 1,500 PK per weekly cycle through tasks. Treat 1,500 as a ceiling, not automatic issuance. Task values and eligibility are configurable. Completion must be server-verified, deduplicated and rate-limited. A player changing timezone must not receive extra cycles. Define one authoritative reset policy and store cycle ID on backend.

Illustrative tasks: daily platform activity, creator discovery, community contribution and weekly objectives. Do not reward harmful engagement, spam, or unsafe behavior. Anti-bot/abuse controls are needed before enabling earning tasks.

## 3. Dynamic reward rate

An economy monitor can adjust future task issuance based on circulation, issuance, sinks, balances, inactivity and transaction volume. Start with a conservative rules-based model, not opaque AI. Publish the current rate and effective date. Cap adjustment per cycle (proposed maximum 5 percentage points or another tested policy) and preserve existing balances. Do not change seller share dynamically without notice.

## 4. Marketplace fee

Baseline direction: seller receives 85% of eligible transaction amount and 15% is platform fee/treasury allocation. Decide rounding policy in whole PK integer units. For a 100 PK sale: buyer -100, seller +85, platform fee +15. Refunds/reversals must reverse the original allocation according to recorded transaction data.

Whether the 15% is permanently removed or split between sink and controlled treasury is OPEN. Do not automatically redistribute all collected currency, as that can create inflation.

## 5. Ledger model

Tables/records: `Wallet`, `LedgerTransaction`, `LedgerEntry`, `MarketplaceListing`, `Purchase`, `OwnershipGrant`, `Refund`, `RewardClaim`, `EconomyCycle`. Prefer double-entry style entries that balance to zero per transaction. Balance cache, if used, must update in the same database transaction as ledger entries or be fully reconstructible.

Transaction fields: transaction ID, idempotency key, type, currency, buyer/seller/system accounts, gross amount, fee, net amount, status, timestamps, reference object and reversal reference.

## 6. Atomic purchase flow

1. Client requests purchase with listing/version and idempotency key.
2. Backend checks account status, listing approval, price, availability, ownership and balance.
3. Backend transaction locks/conditionally updates buyer balance, adds seller/fee ledger entries, and writes ownership grant.
4. Commit once.
5. Client receives receipt.
6. Retry with same idempotency key returns the same result, not a second purchase.

Never trust creator server's statement that a purchase occurred.

## 7. Marketplace categories

Assets, models, textures, audio, animations, UI components, templates, MUTE libraries/modules and tools. Each listing has creator, immutable version, hash, license, dependencies, moderation status, price, payment type (free, one-time PK, or recurring PK if later supported), compatibility and changelog.

## 8. Licensing and disputes

Use explicit license terms: personal/commercial use, modification, redistribution, attribution and exclusivity as applicable. Actual legal license text must be drafted/reviewed. Provide copyright report, ownership dispute, takedown, appeals and repeat infringement handling. Purchased rights after asset removal require a documented policy; never silently change license terms.

## 9. Monthly subscriptions

For a monthly PK subscription, store subscription ID, buyer, listing/version, price, start/end of current period, renewal state, cancellation time, renewal transaction IDs and entitlement status. Renewal uses a unique idempotency key per period. Cancellation stops future renewal but normally keeps access until the already-paid period ends; define exceptions in policy. If the item is removed or seller is suspended, apply the documented refund/entitlement policy. Offline clients may use a cached entitlement only until the stated period/freshness limit; do not mint renewal while offline.

## 10. Refunds and reversals

Define refund eligibility, accidental purchase handling, item removal, creator misconduct and duplicate transaction policies. A reversal is a new compensating ledger transaction; do not delete original entries. If seller has already spent proceeds, the policy must define negative balance/treasury handling.

## 11. Economy monitoring

Track issuance, sinks, circulating balance, active balances, marketplace volume, price index and concentration. Use aggregated data. Avoid penalizing users by reducing existing balances. Keep the reward rate change bounded and transparent.

## 12. Launch gates

Do not enable PK tasks/marketplace until ledger invariants, race-condition tests, idempotency, refund/reversal, account restrictions, fraud monitoring, seller ownership and audit trail are tested. Keep marketplace transactions disabled in early prototype/alpha.
