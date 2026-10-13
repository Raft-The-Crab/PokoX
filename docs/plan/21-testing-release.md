# Testing, Performance, and Release

## Test pyramid
- Unit tests: algorithms, parsers, IDs, transforms, protocol codecs, ledger.
- Property/fuzz tests: packages, binary protocols, bytecode verifier, importers.
- Integration: database/API/auth/publishing/session.
- End-to-end: Studio creator loop, PokoX join/report.
- Performance: renderer, physics, Mute, API, chat, authority.
- Manual: usability, accessibility, moderation workflows.

## Performance method
- [ ] Name device/hardware/software and workload seeds.
- [ ] Measure p50/p95/p99, frame time, allocations, RAM, CPU/GPU and network bytes.
- [ ] Separate warm/cold launch and asset streaming.
- [ ] Compare before/after each optimization.
- [ ] Save profiler traces for regressions.
- [ ] Do not report estimates as observed capacity.

## Baselines to define in Phase 0
Frame time by device tier; memory ceilings; cold launch/join; Mute compile latency; physics step time by body/contact count; API p95/p99; authority memory/latency; asset parse/download time. Choose values after first measurements rather than inventing untested budgets.

## Release gate
- [ ] Clean build from tagged source.
- [ ] Tests pass and no known critical security issues.
- [ ] Migrations/rollback and package compatibility tested.
- [ ] Observability/alerts and backup restore verified.
- [ ] Release notes, known issues, rollout and rollback plan ready.
