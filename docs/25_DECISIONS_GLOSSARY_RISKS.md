# 25 — Decision Register, Glossary, and Risk Log

**Status:** Living document  
**Rule:** Update this file when a decision changes. Do not silently change a wire format, package format, permission, or persisted schema.

## 1. Decision register

| ID | Topic | Baseline | Status |
|---|---|---|---|
| D-001 | Product model | Downloadable UGC game platform: Steam-like distribution, Roblox-like creation, Minecraft-inspired community hosting | DECIDED |
| D-002 | Client targets | Windows 11+ and Android 11+ | DECIDED |
| D-003 | Studio targets | Windows 11+ and Android 11+ | DECIDED |
| D-004 | Mod target | Windows 11+ | DECIDED |
| D-005 | Engine language | C++ | DECIDED |
| D-006 | Windows UI | Rust + egui | DECIDED |
| D-007 | Android UI | Kotlin | DECIDED |
| D-008 | Graphics foundation | Diligent Core | DECIDED |
| D-009 | Initial graphics | OpenGL on Windows; OpenGL ES on Android | DECIDED, compatibility prototype required |
| D-010 | Input | SDL2 | DECIDED, integration test required |
| D-011 | Physics | Custom C++ implementation, incremental and replaceable | DECIDED |
| D-012 | World | Hybrid ECS with hierarchy/object handles | DECIDED |
| D-013 | MUTE syntax | Lua-like blocks with `end` | DECIDED |
| D-014 | MUTE type model | Dynamic runtime + optional static checking | DECIDED |
| D-015 | MUTE arrays | One-based indexing | DECIDED |
| D-016 | MUTE execution | Interpreter first, bytecode VM later | DECIDED |
| D-017 | MUTE native libraries | Excluded from initial release | DECIDED |
| D-018 | MUTE security | Capability-limited sandbox, client/server/shared realms | DECIDED direction |
| D-019 | Networking serialization | Schema-defined Bitstream, not JSON in gameplay path | DECIDED |
| D-020 | Network transport | Select after prototype and security review | OPEN |
| D-021 | Game formats | `.pk`, `.pkx`, `.pkB` | DECIDED names; binary layout OPEN |
| D-022 | Package integrity | Content hashes and platform-signed release manifest | PROPOSED |
| D-023 | Backend | Go modular monolith | PROPOSED baseline |
| D-024 | Primary data | PostgreSQL | PROPOSED baseline |
| D-025 | Cache | Valkey for ephemeral/cache only | PROPOSED |
| D-026 | Asset store | R2 | PROPOSED, quota constrained |
| D-027 | Authentication | Firebase Auth + PokoX authorization | PROPOSED |
| D-028 | Hosting | Community-hosted + limited PokoX-hosted + local development | DECIDED direction |
| D-029 | Community server trust | Registered but untrusted operator; identity is not proof of honest execution | DECIDED principle |
| D-030 | Moderation | Human review for every new public asset and game release | DECIDED principle |
| D-031 | Sexual content | Sexualized/explicit content prohibited across platform | DECIDED principle |
| D-032 | Violence | Fictional violence permitted according to context/intensity/content label | DECIDED direction |
| D-033 | Pokoins | Up to 1,500 PK weekly earning ceiling, task-based; 85/15 marketplace split | DECIDED direction; economics need simulation |
| D-034 | Pokoins cash-out | No real-money conversion/withdrawal initially | PROPOSED |
| D-035 | Offline | Offline-ready, hybrid, online-required modes | DECIDED direction |
| D-036 | Review runtime | Shared engine in separate restricted process from Poko Mod | DECIDED direction |
| D-037 | Creator server SDK | Full gameplay freedom, scoped PokoX capabilities | DECIDED principle |
| D-038 | Initial public chat | Quick Chat/block/report first; voice/unrestricted DMs deferred | PROPOSED |

## 2. Open architecture decisions

1. Exact minimum OpenGL/OpenGL ES feature levels after hardware survey.
2. Diligent Core + SDL2 context ownership/integration on Windows and Android.
3. Transport choice and realtime delivery/reliability model.
4. Bitstream byte layout, integer encoding, alignment and compatibility rules.
5. Physics coordinate conventions, fixed timestep and collision algorithm choices.
6. MUTE exact grammar, table semantics, type semantics, memory management and scheduling.
7. MUTE VM implementation and garbage collection strategy.
8. PPC binary header, canonical manifest, signature implementation and key rotation.
9. Linux creator-server packaging/runtime distribution and host isolation expectations.
10. Poko DataStore quota, consistency and backup/retention tiers.
11. Exact content labels and age/parental policy by launch jurisdiction.
12. Moderator staffing/capacity, escalation and legal incident procedures.
13. Pokoins weekly cycle/reset policy, task rates, inflation formula and treasury/sink allocation.
14. Creator server capacity and PokoX-hosted slot allocation policy.
15. Offline access freshness/revocation behavior and local save conflict semantics.

## 3. Risk log

| Risk | Likelihood/impact | Mitigation |
|---|---|---|
| Project scope overwhelms solo team | High/critical | Vertical slice, defer advanced features |
| Diligent/MSYS2 integration friction | Medium/high | Feasibility milestone before engine expansion |
| Custom physics consumes schedule | High/high | Incremental scope and replaceable interface |
| MUTE sandbox escape | Medium/critical | VM isolation, fuzzing, no native extensions |
| Malformed package exploit | Medium/critical | Bounds-safe parser, fuzzing, isolated process |
| Client cheats | High/high | Server authority, never trust client outcomes |
| Malicious creator server | High/high | Scoped Poko APIs, no platform secrets, client policy enforcement |
| Free service sleeps/quota exhaustion | High/high | Graceful degradation, cache, backups, provider portability |
| Manual review backlog | High/high | Submission caps, staff capacity gate, pause intake |
| Harmful user-generated content | Medium/critical | Human review, restricted chat, reporting, incident response |
| Pokoins inflation/fraud | Medium/high | Ledger, issuance caps, server-verified tasks, simulation |
| Offline save tampering | High/medium | Treat local saves as untrusted; no offline platform rewards |
| Creator copyright dispute | Medium/high | Licenses, provenance, takedown and appeals |
| Android device fragmentation | High/medium | Capability profiles and real device testing |
| Database data loss | Low/critical | Encrypted backups and restore drills |

## 4. Glossary

- **Authority:** Server process that decides canonical state for an online game session.
- **Bitstream:** PokoX schema-defined compact message serialization; not encryption.
- **Capability:** Explicit permission granted to a script/server for a particular operation.
- **Community-hosted:** Server operated on infrastructure controlled by creator/operator.
- **Content hash:** Digest identifying exact package/asset bytes.
- **DataStore:** PokoX-managed game-scoped persistent storage service.
- **MUTE realm:** Client, server, shared or Studio execution context.
- **PPC:** Proposed common Poko Package Container underlying `.pk`/`.pkx`/`.pkB`.
- **Pokoins (PK):** Platform virtual currency.
- **Review Runtime:** Isolated shared-engine process used by Poko Mod to inspect exact submissions.
- **Server registration:** PokoX recognizes a server identity; it does not attest honest operation.
- **Tenant:** Game/developer namespace isolated from other platform tenants.
- **Offline-ready:** Game can run its declared core experience locally after install.

## 5. Change management

Changes to package layout, Bitstream schemas, MUTE semantics, database migrations, permissions or economy rules require an ADR containing context, options, decision, consequences, migration plan and tests. Add a migration note when released content can be affected.
