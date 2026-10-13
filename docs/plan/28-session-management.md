# Session Management Architecture

## Context
Poko needs secure, scalable session management for:
- PokoX client authentication
- Poko Studio creator authentication
- Poko Moderation tool authentication
- Game session validation
- Session revocation (logout, ban, admin action)

## Session Flow

### Authentication Flow
```
1. Client → Backend: Credentials (username/password or OAuth)
2. Backend → Database: Validate credentials
3. Backend → Valkey: Create session
4. Backend → Client: Session token (HTTP-only cookie or Authorization header)
5. Client → Backend: Include session token in requests
6. Backend → Valkey: Validate session token
7. Backend → Client: Grant/deny access
```

### Session Refresh Flow
```
1. Client → Backend: Refresh token
2. Backend → Valkey: Validate refresh token
3. Backend → Valkey: Create new session token
4. Backend → Client: New session token
5. Backend → Valkey: Invalidate old session token
```

### Session Revocation Flow
```
1. Admin Action (ban, password change, logout all)
2. Backend → Valkey: Delete session(s)
3. Backend → Database: Update user state
4. Client → Backend: Next request with invalidated token
5. Backend → Client: 401 Unauthorized
6. Client → User: Redirect to login
```

## Session Token Design

### Token Format: JWT (JSON Web Token)

**Why JWT?**
- Stateless validation (no DB lookup for basic validation)
- Standard, well-supported
- Contains embedded claims
- Can be signed/encrypted

**Token Structure**:
```
Header: {
  "alg": "RS256",
  "typ": "JWT"
}

Payload: {
  "sub": "user_id",
  "iat": 1234567890,
  "exp": 1234571490,
  "jti": "session_id",
  "scope": ["client", "creator"],
  "device_id": "device_fingerprint"
}

Signature: RS256 signature of header + payload
```

**Token Claims**:
- `sub`: User ID (immutable)
- `iat`: Issued at timestamp
- `exp`: Expiration timestamp
- `jti`: Session ID (unique per session)
- `scope`: Permissions/roles
- `device_id`: Device fingerprint for multi-device management

### Token Storage Strategy

**Access Token**:
- Lifetime: 15 minutes
- Storage: HTTP-only cookie (preferred) or Authorization header
- Validation: Cryptographic signature only (fast)

**Refresh Token**:
- Lifetime: 7 days
- Storage: Secure HTTP-only cookie
- Validation: Database lookup (revocable)
- Rotation: New refresh token issued on each refresh

**Session State in Valkey**:
- Key: `session:{session_id}`
- Value: JSON with session metadata
- TTL: 15 minutes (matches access token)
- Fields:
  - user_id
  - device_id
  - created_at
  - last_active
  - ip_address
  - user_agent

## Session Lifecycle

### Creation
1. User authenticates successfully
2. Generate unique session ID (UUID v4)
3. Create JWT access token (15 min expiry)
4. Create refresh token (7 day expiry)
5. Store session in Valkey with TTL
6. Store refresh token in database
7. Return tokens to client

### Validation
1. Extract token from request
2. Verify JWT signature
3. Check token expiration
4. Check if session exists in Valkey
5. Check if user is not banned
6. Grant access if all checks pass

### Refresh
1. Client sends refresh token
2. Verify refresh token in database
3. Check if refresh token is not revoked
4. Generate new access token
5. Generate new refresh token
6. Invalidate old refresh token
7. Update session in Valkey
8. Return new tokens

### Destruction
1. User logs out
2. Delete session from Valkey
3. Delete refresh token from database
4. Clear client-side cookies

### Revocation
1. Admin action (ban, password change, logout all)
2. Delete all sessions for user from Valkey
3. Delete all refresh tokens for user from database
4. Update user state in database
5. All future requests from invalidated sessions fail

## Multi-Device Management

### Device Fingerprinting
- Collect device characteristics:
  - User agent
  - Screen resolution
  - OS version
  - Browser version (if web)
  - Device ID (if mobile)
- Generate hash fingerprint
- Store with session

### Concurrent Session Limits
- Option 1: Unlimited sessions per user
- Option 2: Limit to N concurrent sessions (e.g., 5)
- Option 3: Limit to 1 session per device type

**Recommended**: Limit to 5 concurrent sessions per user

### Session Management UX
- Show active sessions in user settings
- Allow user to revoke specific sessions
- Show session details (device, location, last active)
- "Logout all devices" button

## Security Considerations

### Token Security
- Use RS256 (asymmetric) for signing, not HS256 (symmetric)
- Keep private key secure (environment variable or secret manager)
- Rotate signing keys periodically
- Use short access token lifetime (15 min)
- Use HTTP-only cookies to prevent XSS
- Use Secure flag on cookies (HTTPS only)
- Use SameSite flag to prevent CSRF

### Session Security
- Validate token on every request
- Check session exists in Valkey (revocation check)
- Check user is not banned on every request
- Track IP address and user agent
- Alert on suspicious activity (new device, new location)
- Implement rate limiting on auth endpoints

### Refresh Token Security
- Store refresh tokens in database (revocable)
- One-time use (rotate on refresh)
- Shorter lifetime than access token
- Revoke on password change
- Revoke on suspicious activity

### Session Fixation Prevention
- Regenerate session ID on login
- Regenerate session ID on privilege elevation
- Regenerate session ID on device change

## Database Schema

### Users Table
```sql
CREATE TABLE users (
  id UUID PRIMARY KEY,
  username VARCHAR(50) UNIQUE NOT NULL,
  email VARCHAR(255) UNIQUE NOT NULL,
  password_hash VARCHAR(255) NOT NULL,
  created_at TIMESTAMP NOT NULL,
  updated_at TIMESTAMP NOT NULL,
  banned BOOLEAN DEFAULT FALSE,
  ban_reason TEXT,
  ban_expires_at TIMESTAMP
);
```

### Refresh Tokens Table
```sql
CREATE TABLE refresh_tokens (
  id UUID PRIMARY KEY,
  user_id UUID NOT NULL REFERENCES users(id),
  token_hash VARCHAR(255) UNIQUE NOT NULL,
  device_id VARCHAR(255),
  created_at TIMESTAMP NOT NULL,
  expires_at TIMESTAMP NOT NULL,
  revoked BOOLEAN DEFAULT FALSE,
  revoked_at TIMESTAMP,
  revoked_reason VARCHAR(255)
);
```

### Sessions Table (for audit/logging)
```sql
CREATE TABLE sessions (
  id UUID PRIMARY KEY,
  user_id UUID NOT NULL REFERENCES users(id),
  device_id VARCHAR(255),
  ip_address INET,
  user_agent TEXT,
  created_at TIMESTAMP NOT NULL,
  expires_at TIMESTAMP NOT NULL,
  destroyed_at TIMESTAMP,
  destruction_reason VARCHAR(255)
);
```

## Valkey Data Structures

### Session Storage
```
Key: session:{session_id}
Type: Hash
TTL: 900 seconds (15 minutes)
Fields:
  user_id: UUID
  device_id: string
  ip_address: string
  user_agent: string
  created_at: timestamp
  last_active: timestamp
```

### User Sessions Index
```
Key: user_sessions:{user_id}
Type: Set
TTL: 604800 seconds (7 days)
Members: session_ids
```

### Rate Limiting
```
Key: ratelimit:{user_id}:{endpoint}
Type: String
TTL: 60 seconds
Value: request_count
```

## API Endpoints

### POST /auth/login
- Request: `{ username, password, device_info }`
- Response: `{ access_token, refresh_token, expires_in }`
- Headers: `Set-Cookie: access_token=...; HttpOnly; Secure; SameSite=Strict`

### POST /auth/refresh
- Request: `{ refresh_token }`
- Response: `{ access_token, refresh_token, expires_in }`
- Headers: `Set-Cookie: access_token=...; HttpOnly; Secure; SameSite=Strict`

### POST /auth/logout
- Request: (authenticated)
- Response: `{ success: true }`
- Action: Delete session, delete refresh token

### GET /auth/sessions
- Request: (authenticated)
- Response: `{ sessions: [{ id, device, location, last_active }] }`

### DELETE /auth/sessions/{session_id}
- Request: (authenticated)
- Response: `{ success: true }`
- Action: Revoke specific session

### DELETE /auth/sessions
- Request: (authenticated)
- Response: `{ success: true }`
- Action: Revoke all sessions except current

## Performance Considerations

### Caching
- Cache user data in Valkey (reduce DB lookups)
- Cache permissions/roles in Valkey
- Cache ban status in Valkey (TTL: 1 minute)

### Connection Pooling
- Database connection pool size: 10-20 connections
- Valkey connection pool size: 10-20 connections
- Reuse connections efficiently

### Batch Operations
- Batch session lookups where possible
- Batch refresh token validation
- Use Valkey pipelines for multiple operations

## Monitoring

### Metrics
- Session creation rate
- Session validation rate
- Session refresh rate
- Session destruction rate
- Failed session validations (expired, revoked, banned)
- Average session duration
- Concurrent sessions per user

### Alerts
- High session creation rate (possible attack)
- High failed validation rate (possible attack)
- High session destruction rate (possible logout storm)
- Database connection pool exhaustion
- Valkey memory pressure

## Implementation Order

1. **Phase 1**: Basic auth flow (login, validate, logout)
2. **Phase 2**: Refresh token mechanism
3. **Phase 3**: Session revocation (ban, password change)
4. **Phase 4**: Multi-device management
5. **Phase 5**: Session management UI
6. **Phase 6**: Security hardening and monitoring

## Revisit Triggers
- Token validation becomes bottleneck
- Session storage exceeds Valkey capacity
- Security incident related to sessions
- New requirements (SSO, MFA, etc.)

## References
- JWT best practices (RFC 8725)
- OWASP Session Management Cheat Sheet
- OAuth 2.0 RFC 6749
- RFC 6265 (HTTP State Management Mechanism)
