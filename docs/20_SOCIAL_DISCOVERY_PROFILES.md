# 20 — Player Profiles, Social, Discovery, and Inventory

**Status:** Proposed product baseline  
**Safety posture:** Conservative social functionality at launch.

## 1. Player profile

Profile fields: internal UserID (not public), username, display name, avatar reference, biography (filtered), join date (optional visibility), creator games, favorites, badges, privacy settings and social settings. Never expose email, Firebase UID, auth token, IP address, exact birthdate or sensitive internal identifiers.

## 2. Avatar and platform inventory

Platform inventory contains approved avatar items, emotes and platform badges. Game-specific inventory remains inside the game/DataStore. Asset ownership includes purchase/entitlement record, license version, creator, asset version and moderation state. Uploaded avatar assets go through the same human review policy.

## 3. Friends and parties

Features: friend request, accept/decline, remove, block, mute, invite, presence visibility and join-friend where the game/host permits. Privacy settings must override social convenience. Blocking should prevent direct contact and repeated interaction requests to the extent controlled by platform. Do not reveal who blocked a user unnecessarily.

Parties are platform session groups, not game authority. When joining a game, backend checks game access, content suitability, privacy, server capacity and party size.

## 4. Communication MVP

Start with curated Quick Chat. Defer voice and unrestricted private messages. If filtered text chat launches later, route through a platform service with moderation, abuse detection, rate limits, reporting and personal-information handling. Game servers cannot bypass chat policy by sending arbitrary client UI text as a communication channel.

## 5. Game discovery

Sections: recommended, popular, new releases, recently updated, categories, favorites and friends playing (subject to privacy). Search by game title, creator, tags and category. Only approved, currently distributable release versions appear publicly.

## 6. Ranking

Start with understandable rules-based ranking: category relevance, popularity, recency, favorites, editorial/curated placement, suitability and safety status. Do not optimize only for time spent. Do not use sensitive child behavior profiles. Provide controls to hide/report content and explain sponsored/featured placement if ever introduced.

## 7. Game page

Show title, creator, description, screenshots, video only if safely moderated, content label/descriptors, platform compatibility, download/installed size, offline status, current version, update date, server availability, player count if reliable, privacy/social features and report action.

## 8. Game and platform inventory separation

PokoX inventory and game-defined inventory are separate namespaces. Game cannot create platform-owned avatar items or badges unless a PokoX service authorizes the grant. Offline achievements remain local/unverified until accepted.

## 9. Privacy requirements

Presence can be hidden. Profile visibility can be restricted. Do not show server IP or network endpoint details. Use game-scoped pseudonymous IDs in creator APIs where practical to limit cross-game tracking. Document data collection and retention.

## 10. Acceptance tests

- Blocked players cannot send supported invites/messages.
- Private presence is not exposed.
- Unapproved game does not appear in search.
- Content label appears before launch.
- Platform inventory cannot be changed by game script.
- Player profile does not expose private account fields.
- Discovery ranking excludes suspended/revoked releases.
