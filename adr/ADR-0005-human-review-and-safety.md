# ADR-0005 — Human Review Before Public Distribution

**Status:** Accepted baseline  
**Date:** 2026-11-03

## Context
PokoX accepts user-generated games, assets, animations, audio and code. Its intended audience includes children.

## Decision
Every new public asset/library version and every public game release is human-reviewed before distribution. Changed content requires re-review; exact unchanged hash-bound dependencies may reuse prior approval if policy allows. Sexualized/sexually explicit content is prohibited. Fictional violence and bounded stylized gore may be allowed by PokoX content label after human review. Poko Mod uses an isolated Review Runtime.

## Consequences
- Publishing capacity must be capped by moderator staffing.
- Queue overload pauses intake rather than auto-approving.
- Safety incident and legal escalation processes are required before public launch.
