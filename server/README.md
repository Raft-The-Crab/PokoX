# Creator Server SDK and Runtime

Official C++ headless runtime and SDK for creator-hosted game servers.

## Components

- **registration**: Server key generation, registration, and public key management
- **credentials**: Short-lived credential issuance, rotation, and revocation
- **heartbeat**: Server health monitoring and session status
- **sdk**: Poko Services API client (DataStore, Messaging, Matchmaking, etc.)
- **messaging**: Cross-server communication
- **datastore**: Game-scoped persistent storage client

## Trust Model

- Creator servers are untrusted operators for platform-wide claims
- Registration proves identity/credential possession, not honest execution
- Platform services enforce scope independently
- No access to platform secrets, Pokoins ledger, or moderation admin

## Permissions

Scoped permissions per game and server instance:
- Basic: Session management, world access, physics queries
- Extended: DataStore, Messaging, Matchmaking, Teleport, Chat, Analytics
- Sensitive: Requires separate PokoX review and narrow purpose

## Deployment

- Linux x86-64 runtime distribution
- Requires PokoX-approved package hashes
- Each server instance has unique key pair
- Credentials are short-lived and revocable
