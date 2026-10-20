---
id: "docs-08-marketplace-commerce-and-entitlements-md"
title: "Marketplace and Real-Money Commerce"
kind: "implementation-contract"
execution_phase: "deferred-online"
status: "specification"
source_of_truth: "This file plus ../02-architecture/module-boundaries.md and ../11-engineering/testing-and-ci.md"
---

# Agent Contract: Marketplace and Real-Money Commerce

## Execution directive

Treat this document as an implementation contract, not as permission to implement the entire product in one pass. Work only on the smallest independently testable slice that is unblocked by the repository's current state.

1. Inspect the repository before editing. Reuse existing conventions and code; do not assume this plan describes code that already exists.
2. Identify the current milestone in `STATUS.md` and the next unchecked task in `TASKS.md`.
3. Read this contract and its linked dependency contracts before changing code.
4. Write a short implementation plan in the task response: files to touch, interfaces, tests, and risks. Then implement; do not stop at planning.
5. Keep changes narrowly scoped. Do not add speculative frameworks, placeholder implementations presented as complete, or unrelated features.
6. Build and run the narrowest relevant tests, then run the broader available test suite.
7. Update `STATUS.md` and `TASKS.md` only when the code and verification evidence justify the status change.
8. If a decision is unresolved, record the options and evidence in `docs/decisions/open-decisions.md`; do not silently invent a permanent architectural decision.
9. Never claim a build/test passed unless you actually ran it and observed success. If blocked by environment/tooling, report the exact blocker and the command not run.
10. Keep secrets, credentials, signing keys, private user data, and production endpoints out of source control.

## Scope

- **Owns:** Marketplace and Real-Money Commerce
- **Document:** `docs/08-marketplace/commerce-and-entitlements.md`
- **Execution phase:** `deferred-online`
- **Do not implement yet:** online/commerce/multiplayer/security-sensitive production features unless their prerequisite gates are met.

## Required implementation sequence

1. Establish current repository state and existing implementation.
2. Extract one concrete behavior or interface from the requirements below.
3. Define input/output contracts and error behavior before implementation.
4. Implement the minimum vertical slice.
5. Add tests before marking the slice complete.
6. Verify compilation, tests, and relevant platform constraints.
7. Update the task ledger with evidence and any remaining blocker.

## Acceptance contract

- [ ] Implementation matches the requirements defined in `docs/08-marketplace/commerce-and-entitlements.md`.
- [ ] Public interfaces and ownership boundaries are explicit.
- [ ] Failure cases are handled deterministically and produce actionable diagnostics.
- [ ] Automated tests cover normal behavior, invalid input, and boundary conditions.
- [ ] The change builds and tests on the currently supported target.

## Implementation constraints

- Prefer deterministic behavior and explicit ownership/lifetimes.
- Validate external, serialized, and user-controlled input at the boundary.
- Avoid unnecessary dependencies; pin any dependency that is introduced and document its license.
- Keep platform-specific code behind platform interfaces.
- Do not expose renderer, OS, networking, or scripting implementation details through unrelated modules.
- Add or update tests for every behavior change; include negative and boundary tests where applicable.
- Preserve backward compatibility deliberately. If a format/API changes, add a versioning or migration plan.
- A TODO is not acceptance evidence. A stub is not a completed feature.

## Requirements and design detail

The following requirements are preserved from the source plan. They are binding unless superseded by a recorded decision.

## Launch rule

Real-money marketplace is a product goal, but it must not be enabled until provider, legal, security, moderation, support, tax, refund, and payout readiness are established. Free content can launch earlier.

## Product model

- Product listing
- Immutable product version
- Price and currency
- Seller/creator identity
- Order
- Payment-provider reference
- Entitlement/license
- Refund/dispute
- Creator ledger
- Payout record
- Moderation state

## Purchase flow

1. User selects a product.
2. Backend checks eligibility and creates checkout/session.
3. Provider processes payment.
4. Backend verifies a signed provider event.
5. Idempotent order processing records the payment.
6. Entitlement is granted.
7. Download/use is authorized by backend policy.
8. Refund/dispute events update the ledger and entitlement state.

Never grant entitlements based only on a client redirect or client-submitted claim.

## Accounting

- Use integer minor units for money.
- Store explicit currency.
- Keep gross, provider fees, platform fees, refunds, adjustments, and creator payable amounts separate.
- Use an immutable ledger with reconciliation.
- Make provider webhooks idempotent.
- Keep payout actions auditable.
- Do not implement a custom payment processor.

## License rules

Define behavior for:
- Continued use after purchase
- Refunds
- Subscription expiry if subscriptions are introduced
- Unpublishing
- Creator suspension
- Takedown/legal complaints
- Package version updates
- Offline access and cached copies

## Pokoins

Do not introduce Pokoins until a secure immutable ledger, atomic operations, abuse controls, and policy are ready. Any proposed weekly reward limits, taxes, or circulation controls are hypotheses, not fixed policy. Model and test them before adoption.

## Release gate

Commerce must not launch until provider terms, regional availability, age requirements, tax obligations, creator payouts, refund/dispute workflows, fraud controls, and support escalation are reviewed by qualified people.
