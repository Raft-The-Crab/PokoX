# 36 — Finalization Register and Open Technical Gates

This is the final planning closeout. It distinguishes product decisions that are now settled from details that must be established empirically before implementation is locked.

## 1. Settled product/architecture decisions

| Topic | Baseline decision |
|---|---|
| Platform identity | Downloadable UGC game platform combining game launcher, creator ecosystem and community hosting |
| Applications | PokoX Client; Poko Studio; Poko Mod |
| Engine | Shared C++ engine; Windows-first, Android-compatible architecture |
| Graphics | Diligent Core abstraction; OpenGL Windows baseline; OpenGL ES Android baseline |
| Input | SDL2 |
| Physics | Custom incremental implementation behind replaceable interface |
| Windows UI | Rust + egui |
| Android UI | Kotlin |
| Backend | Go modular monolith |
| Identity | Firebase authentication + PokoX internal identity/authorization |
| MUTE | Lua-style, `end`, dynamic runtime + optional static analysis, 1-based arrays, interpreter first, VM later, no native libraries initially |
| MUTE security | Client/Server/Shared/Studio realms, capability system, execution budgets |
| Network encoding | Schema-defined Bitstream; transport to be proven in prototype |
| Hosting | Local, community-hosted, limited PokoX-hosted |
| Creator control | Extensive game authority; no platform-admin powers |
| Packages | `.pk` project, `.pkx` player, `.pkB` backend; signed PPC container |
| Review | Mandatory human review before public publication |
| Violence policy | Fictional combat and bounded stylized gore may be permitted by label/review |
| Sexual content | Sexualized/sexually explicit content prohibited throughout platform |
| Offline | OFFLINE_READY, HYBRID, ONLINE_REQUIRED; local saves untrusted; no offline Pokoins |
| Poko Mod | Shared engine in isolated Review Runtime process |
| Economy | Virtual Pokoins; proposed 1,500 weekly earning cap; seller 85%, platform 15%; ledger-based |
| Rilavek | Optional TUS/resumable ingestion into R2; not storage/CDN |
| HF deployment | Account A Go / Account B C++; conditional on independent compute eligibility and transport proof |

## 2. Open technical gates (must resolve with prototypes)

1. Diligent Core + SDL2 context and swap-chain ownership in MSYS2 UCRT64.
2. Android NDK integration and actual GL ES capabilities.
3. Physics broadphase/narrowphase algorithms and tolerances.
4. MUTE grammar edge cases, bytecode design and VM instruction semantics.
5. Exact capability representation and strongest practical sandbox isolation.
6. Bitstream header, encoding, sequence/replay rules and real-time transport.
7. Ed25519 key lifecycle, canonical manifest encoding, signatures and rotation procedure.
8. PPC binary layout, compression limits, chunk sizing and package parser threat model.
9. Database schema/indexes and DataStore quota numbers from load tests.
10. Creator server heartbeat, join-ticket consumption and reconnect protocol.
11. HF Docker eligibility per account and actual inbound authority transport.
12. Rilavek vendor/privacy review and upload quota test before production use.
13. Pokoins issuance rate, sink fraction and dynamic reward adjustment after economy simulation.
14. Exact local age/privacy configuration and child-safety legal process per launch jurisdiction.
15. Moderation staffing and maximum submissions/day based on measured review throughput.

## 3. Defaults for unresolved choices

If a prototype requires a choice before a formal ADR is written, use the simplest portable default: fixed-step simulation; HTTPS control plane; authenticated secure real-time transport selected by proof-of-concept; immutable release hashes; conditional writes for persistent data; server-side authorization; no hidden/unreviewed package loading; no automatic permanent bans based on a single heuristic; no dual-write databases.

## 4. What this plan does not claim

- The project has not been built or validated.
- The free provider setup is not proven as a reliable public production hosting solution.
- A creator server cannot be proven honest solely by using the SDK.
- Client assets cannot be made impossible to extract once delivered to a device.
- Anti-cheat cannot guarantee zero cheating.
- The content labels are PokoX labels, not official regional ratings.
- This plan is not legal advice or a compliance certification.

## 5. Planning closeout

This document set is the consolidated baseline for implementation. New feature ideas should be logged as backlog items and assessed against project capacity. Architecture changes to package, protocol, trust, permission, data ownership or economy invariants require an ADR. The next work item is M0 feasibility, not more feature expansion.
