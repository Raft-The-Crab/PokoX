# 18 — Security, Anti-Cheat, and Abuse Resistance

**Status:** Security baseline; detailed threat model and penetration tests required.  
**Objective:** Cheat-resistant by architecture, not a claim of perfect anti-cheat.

## 1. Threat actors

- Player with modified client, memory editor, packet tool, bots or stolen account.
- Malicious creator distributing unsafe assets/scripts.
- Malicious community server operator changing code after approval.
- Compromised creator credentials or server key.
- Malicious marketplace library author.
- Abusive moderator or compromised moderator account.
- External attacker targeting APIs, storage or infrastructure.

## 2. Assets to protect

Accounts and sessions, Pokoins and purchase history, marketplace ownership, private user data, creator projects, approved release records, moderation evidence, server credentials, DataStore namespaces, platform signing keys, and service availability.

## 3. Trust principles

- Client is untrusted.
- Community-hosted server operator is untrusted.
- Creator scripts and libraries are untrusted.
- Downloaded packages are untrusted until verified.
- Moderator UI is trusted only after strong authentication and least-privilege checks.
- PokoX platform services own platform records and transactions.

## 4. Gameplay anti-cheat

Server authority validates movement, speed/acceleration, collision, jump/fly modes, interaction range, cooldowns, combat, health, inventory, abilities and state transitions according to each game's server-side rules. Do not enforce one universal movement rule that breaks legitimate genres. Client prediction improves responsiveness but never owns the final online state.

## 5. Network protection

Authenticated encrypted transport, versioned Bitstream schemas, payload size limits, field/range checks, sequence/replay handling, per-message quotas, rate limits, connection/session binding, timeouts and server-side state validation. A checksum is not authentication. Bitstream is not encryption.

## 6. Client integrity

Verify package signatures and chunk hashes. Use selective runtime integrity checks as telemetry/defense-in-depth, not sole enforcement. A determined player can patch user-mode software. Do not rely on anti-debugging or obfuscation as a security boundary. Keep sensitive server logic out of `.pkx`.

## 7. Creator/server threats

Community operators can modify server code and fabricate server claims. Protect players using client-enforced approved asset manifests, platform-mediated communication, platform-authorized purchases and reports, and no creator access to sensitive account data. Treat community-server achievements, analytics and progression claims as untrusted for platform-wide rewards. PokoX-hosted sessions can offer stronger execution assurance, but still sandbox creator scripts.

## 8. Platform API security

Apply authentication, object-level authorization, function-level authorization, property-level authorization, validation, resource quotas, idempotency, audit logging and secret management to every endpoint. Never assume an SDK wrapper is a security boundary. Test direct crafted requests. Avoid SSRF by not accepting arbitrary server-side fetch URLs; use allowlists and egress controls for any future HTTP proxy.

## 9. Economy security

Pokoins balance is derived from a platform-controlled ledger or maintained transactionally with a ledger invariant. Creator server cannot mint Pokoins. Purchases are atomic and idempotent. Prevent replay, duplicate grant and race conditions. Use explicit refunds/reversals rather than editing history.

## 10. Account and moderator security

Firebase token validation plus PokoX authorization. Short-lived sessions, refresh rotation, revocation, login rate limits, MFA for moderators/admins, scoped roles, step-up auth for destructive actions, and audit logs. Moderator activation keys are one-time random values; store only a secure verifier/hash and redeem atomically. The activation key is not a permanent login credential.

## 11. Detection and enforcement

Use layered actions: log, reject invalid action, rate-limit, monitor, restrict session, disconnect, quarantine, human review, sanction. Risk scoring can prioritize review but weak heuristics must not trigger permanent bans alone. Record reason, evidence, policy and appeal path.

## 12. Logging and privacy

Log identifiers required to investigate events, not passwords, auth tokens, full payment secrets, unnecessary chat content or precise personal data. Apply retention schedules and role-based access. Restrict evidence access and record who opened it.

## 13. Security testing

Threat modeling, code review, dependency scanning, secret scanning, fuzzing, static analysis, integration tests, cross-tenant tests, abuse simulation and external review before public launch. Prioritize package parser, MUTE VM, Bitstream decoder, auth/session code, economy ledger and creator service authorization.

## 14. Acceptance gates

No public multiplayer beta until: impossible actions are rejected; server identity/revocation works; cross-game access is denied; Pokoins cannot be granted by client or creator server; replay/duplicate transactions are handled; package parser is fuzzed; reports are independent of game servers; and incident response is documented.

## 15. References

- [OWASP Game Security Framework](https://owasp.org/projects/gamesec-framework)
- [OWASP API Security Top 10](https://owasp.org/www-project-api-security/)
- [OWASP ASVS](https://owasp.org/www-project-application-security-verification-standard/)
