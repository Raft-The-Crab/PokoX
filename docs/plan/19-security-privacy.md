# Security, Privacy, and Abuse

## Threat model
For each feature record attacker, asset, entry point, trust boundary, likely abuse, detection and mitigation.

## Minimum controls
- [ ] Secrets outside source/client packages.
- [ ] TLS and explicit authentication/origin policy.
- [ ] Per-route and object-level authorization.
- [ ] Rate limits and bounded payloads.
- [ ] Validate all external input; parameterized DB access.
- [ ] Dependency pinning, license review and vulnerability scanning.
- [ ] Package/update verification and key rotation plan.
- [ ] Session revocation and privileged-action audit.
- [ ] Protected backups and restore tests.

## Mute sandbox
- [ ] No native pointer access.
- [ ] Capability allowlist and bytecode verifier.
- [ ] CPU/instruction, memory/object and recursion limits.
- [ ] Cancellation/deadlines.
- [ ] Filesystem/network default-deny.
- [ ] Script error isolation where feasible.

## Anti-cheat
- [ ] Document client-only presentation state.
- [ ] Validate consequential actions server-side.
- [ ] Cooldowns, ownership/range checks and rate limits where appropriate.
- [ ] Do not promise detection of every cheat.
- [ ] Collect only useful defense signals and document retention.
- [ ] Provide false-positive review/appeal path.

## Privacy
- [ ] Data inventory and purpose.
- [ ] Minimize collection; define retention/deletion and access roles.
- [ ] Do not expose private user data to game scripts or Studio logs.
- [ ] Review applicable legal obligations with qualified advice before launch.
