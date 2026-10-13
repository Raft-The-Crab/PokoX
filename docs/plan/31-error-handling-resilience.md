# Error Handling and Client Resilience Strategy

## Context
Poko clients need to handle network failures gracefully:
- PokoX client (Windows/Android)
- Game sessions (real-time multiplayer)
- Studio (offline-capable editor)
- Need reliable user experience despite network issues

## Error Classification

### Error Types

#### 1. Transient Errors (Retryable)
- Network timeout
- Connection refused
- 503 Service Unavailable
- 504 Gateway Timeout
- Rate limit exceeded (429)
- Temporary server error

#### 2. Permanent Errors (Non-Retryable)
- 400 Bad Request
- 401 Unauthorized
- 403 Forbidden
- 404 Not Found
- 409 Conflict
- 422 Unprocessable Entity
- Client-side validation errors

#### 3. Fatal Errors (User Action Required)
- 500 Internal Server Error (persistent)
- Database connection failure (persistent)
- Authentication failure (invalid credentials)
- Session expired
- Account banned
- Version mismatch

## Error Response Format

### Standard Error Response
```json
{
  "error": {
    "code": "AUTH_INVALID_CREDENTIALS",
    "message": "Invalid username or password",
    "details": {
      "field": "username"
    },
    "request_id": "abc123",
    "timestamp": "2024-01-15T10:30:00Z"
  }
}
```

### Error Codes
- **AUTH_***: Authentication errors
- **SESSION_***: Session errors
- **RATE_LIMIT_***: Rate limiting errors
- **VALIDATION_***: Input validation errors
- **PERMISSION_***: Authorization errors
- **NOT_FOUND_***: Resource not found errors
- **CONFLICT_***: Conflict errors
- **INTERNAL_***: Internal server errors
- **NETWORK_***: Network errors

## Retry Strategy

### Exponential Backoff with Jitter
```rust
// Retry configuration
const MAX_RETRIES: u32 = 3;
const INITIAL_DELAY_MS: u64 = 100;
const MAX_DELAY_MS: u64 = 5000;
const JITTER_MS: u64 = 100;

async fn retry_with_backoff<F, Fut, T, E>(operation: F) -> Result<T, E>
where
    F: Fn() -> Fut,
    Fut: Future<Output = Result<T, E>>,
    E: std::error::Error,
{
    let mut delay = INITIAL_DELAY_MS;
    for attempt in 0..=MAX_RETRIES {
        match operation().await {
            Ok(result) => return Ok(result),
            Err(e) if is_transient_error(&e) && attempt < MAX_RETRIES => {
                let jitter = rand::random::<u64>() % JITTER_MS;
                tokio::time::sleep(Duration::from_millis(delay + jitter)).await;
                delay = (delay * 2).min(MAX_DELAY_MS);
            }
            Err(e) => return Err(e),
        }
    }
    Err(operation().await.unwrap_err())
}
```

### Retryable Error Detection
```rust
fn is_transient_error(error: &dyn std::error::Error) -> bool {
    match error.downcast_ref::<HttpError>() {
        Some(HttpError::Timeout) => true,
        Some(HttpError::ServiceUnavailable) => true,
        Some(HttpError::GatewayTimeout) => true,
        Some(HttpError::RateLimitExceeded) => true,
        _ => false,
    }
}
```

### Retry Configuration per Endpoint
| Endpoint | Max Retries | Initial Delay | Max Delay |
|-----------|-------------|---------------|-----------|
| Auth/login | 2 | 100ms | 1s |
| Auth/refresh | 3 | 100ms | 2s |
| API/* | 3 | 100ms | 5s |
| Game/* | 1 | 50ms | 100ms |
| Asset/download | 5 | 200ms | 10s |

## Client Error Handling

### PokoX Client (Rust Engine)

#### API Request Handling
```rust
pub async fn api_request<T>(endpoint: &str, payload: T) -> Result<Response, Error> {
    retry_with_backoff(|| async {
        let response = http_client
            .post(endpoint)
            .json(&payload)
            .send()
            .await?;

        if response.status().is_success() {
            Ok(response)
        } else {
            let error = response.json::<ErrorResponse>().await?;
            Err(Error::Api(error))
        }
    }).await
}
```

#### Error Display to User
```rust
fn display_error(error: &Error) -> String {
    match error {
        Error::Network => "Network error. Please check your connection.".to_string(),
        Error::Api(ref e) if e.code == "AUTH_INVALID_CREDENTIALS" => {
            "Invalid username or password".to_string()
        }
        Error::Api(ref e) if e.code == "RATE_LIMIT_EXCEEDED" => {
            "Too many requests. Please try again later.".to_string()
        }
        Error::Api(ref e) => e.message.clone(),
        Error::Internal => "An unexpected error occurred. Please try again.".to_string(),
    }
}
```

#### Network State Monitoring
```rust
// Monitor network connectivity
pub struct NetworkMonitor {
    last_success: Instant,
    failure_count: u32,
}

impl NetworkMonitor {
    pub fn report_success(&mut self) {
        self.last_success = Instant::now();
        self.failure_count = 0;
    }

    pub fn report_failure(&mut self) {
        self.failure_count += 1;
    }

    pub fn is_network_degraded(&self) -> bool {
        self.failure_count > 3
            || self.last_success.elapsed() > Duration::from_secs(30)
    }
}
```

### Game Session Resilience

#### WebSocket Connection Handling
```rust
pub struct GameConnection {
    ws: WebSocket,
    reconnect_timer: Option<Instant>,
    state: ConnectionState,
}

enum ConnectionState {
    Connected,
    Disconnected,
    Reconnecting,
    Failed,
}

impl GameConnection {
    pub async fn connect(&mut self) -> Result<(), Error> {
        loop {
            match WebSocket::connect(&self.game_url).await {
                Ok(ws) => {
                    self.ws = ws;
                    self.state = ConnectionState::Connected;
                    return Ok(());
                }
                Err(e) if is_transient_error(&e) => {
                    self.state = ConnectionState::Reconnecting;
                    tokio::time::sleep(Duration::from_secs(5)).await;
                }
                Err(e) => {
                    self.state = ConnectionState::Failed;
                    return Err(e);
                }
            }
        }
    }

    pub async fn send_message(&mut self, message: Message) -> Result<(), Error> {
        match self.state {
            ConnectionState::Connected => {
                self.ws.send(message).await.map_err(Error::Network)
            }
            ConnectionState::Disconnected | ConnectionState::Reconnecting => {
                Err(Error::NotConnected)
            }
            ConnectionState::Failed => {
                Err(Error::ConnectionFailed)
            }
        }
    }
}
```

#### State Synchronization
```rust
// Optimistic updates with rollback
pub struct GameState {
    local_state: HashMap<EntityId, EntityState>,
    server_state: HashMap<EntityId, EntityState>,
    pending_updates: Vec<EntityUpdate>,
}

impl GameState {
    pub fn update_local(&mut self, entity_id: EntityId, new_state: EntityState) {
        self.pending_updates.push(EntityUpdate {
            entity_id,
            old_state: self.local_state.get(&entity_id).cloned(),
            new_state: new_state.clone(),
        });
        self.local_state.insert(entity_id, new_state);
    }

    pub fn confirm_update(&mut self, entity_id: EntityId) {
        self.server_state.insert(entity_id, self.local_state[&entity_id].clone());
        self.pending_updates.retain(|u| u.entity_id != entity_id);
    }

    pub fn rollback_update(&mut self, entity_id: EntityId) {
        if let Some(update) = self.pending_updates.iter().find(|u| u.entity_id == entity_id) {
            if let Some(old_state) = &update.old_state {
                self.local_state.insert(entity_id, old_state.clone());
            }
        }
    }
}
```

#### Reconnection Strategy
```rust
pub async fn reconnect_with_state_sync(&mut self) -> Result<(), Error> {
    // 1. Save current state
    let state_snapshot = self.game_state.clone();

    // 2. Attempt reconnection
    self.connect().await?;

    // 3. Request server state
    let server_state = self.request_server_state().await?;

    // 4. Reconcile state
    self.reconcile_state(state_snapshot, server_state).await?;

    Ok(())
}

async fn reconcile_state(&mut self, local: GameState, server: GameState) -> Result<(), Error> {
    // Apply server state as truth
    self.game_state = server;

    // Re-send pending updates that weren't confirmed
    for update in local.pending_updates {
        self.send_message(Message::Update(update)).await?;
    }

    Ok(())
}
```

### Studio Resilience

#### Offline Mode
```rust
pub struct Studio {
    local_projects: HashMap<ProjectId, Project>,
    sync_queue: Vec<SyncOperation>,
    online: bool,
}

impl Studio {
    pub fn save_locally(&mut self, project: Project) {
        self.local_projects.insert(project.id, project.clone());
        self.sync_queue.push(SyncOperation::Save(project));
    }

    pub async fn sync_when_online(&mut self) -> Result<(), Error> {
        if !self.online {
            return Ok(());
        }

        for operation in self.sync_queue.drain(..) {
            match operation {
                SyncOperation::Save(project) => {
                    self.api.save_project(&project).await?;
                }
                SyncOperation::Delete(project_id) => {
                    self.api.delete_project(project_id).await?;
                }
            }
        }

        Ok(())
    }
}
```

#### Auto-Save
```rust
pub struct AutoSave {
    interval: Duration,
    last_save: Instant,
}

impl AutoSave {
    pub fn should_save(&self) -> bool {
        self.last_save.elapsed() > self.interval
    }

    pub fn mark_saved(&mut self) {
        self.last_save = Instant::now();
    }
}
```

## Server Error Handling

### Rust Backend Error Handling
```rust
#[derive(Debug)]
pub enum ApiError {
    ValidationError(String),
    AuthenticationError(String),
    AuthorizationError(String),
    NotFound(String),
    Conflict(String),
    RateLimitExceeded(String),
    InternalError(String),
}

impl IntoResponse for ApiError {
    fn into_response(self) -> Response {
        let (status, code, message) = match self {
            ApiError::ValidationError(msg) => (StatusCode::BAD_REQUEST, "VALIDATION_ERROR", msg),
            ApiError::AuthenticationError(msg) => (StatusCode::UNAUTHORIZED, "AUTH_ERROR", msg),
            ApiError::AuthorizationError(msg) => (StatusCode::FORBIDDEN, "PERMISSION_ERROR", msg),
            ApiError::NotFound(msg) => (StatusCode::NOT_FOUND, "NOT_FOUND", msg),
            ApiError::Conflict(msg) => (StatusCode::CONFLICT, "CONFLICT", msg),
            ApiError::RateLimitExceeded(msg) => (StatusCode::TOO_MANY_REQUESTS, "RATE_LIMIT", msg),
            ApiError::InternalError(msg) => (StatusCode::INTERNAL_SERVER_ERROR, "INTERNAL_ERROR", msg),
        };

        let error_response = ErrorResponse {
            error: ErrorDetail {
                code: code.to_string(),
                message,
                details: None,
                request_id: uuid::Uuid::new_v4().to_string(),
                timestamp: Utc::now(),
            },
        };

        (status, Json(error_response)).into_response()
    }
}
```

### Game Handler Error Handling
```rust
pub async fn handle_game_message(message: GameMessage) -> Result<GameResponse, GameError> {
    match message {
        GameMessage::Move { entity_id, position } => {
            validate_move(entity_id, position)?;
            apply_move(entity_id, position).await?;
            Ok(GameResponse::Success)
        }
        GameMessage::Interact { entity_id, target_id } => {
            validate_interaction(entity_id, target_id)?;
            apply_interaction(entity_id, target_id).await?;
            Ok(GameResponse::Success)
        }
        GameMessage::Invalid => {
            Err(GameError::InvalidMessage)
        }
    }
}
```

### Rate Limiting with Error Response
```rust
pub async fn rate_limit_middleware(
    req: Request,
    next: Next,
) -> Result<Response, StatusCode> {
    let user_id = req.extensions().get::<UserId>().unwrap();

    if !check_rate_limit(user_id).await {
        return Err(StatusCode::TOO_MANY_REQUESTS);
    }

    Ok(next.run(req).await)
}
```

## Circuit Breaker Pattern

### Circuit Breaker Implementation
```rust
pub struct CircuitBreaker {
    failure_count: u32,
    failure_threshold: u32,
    last_failure_time: Instant,
    cooldown: Duration,
    state: CircuitState,
}

enum CircuitState {
    Closed,
    Open,
    HalfOpen,
}

impl CircuitBreaker {
    pub async fn call<F, Fut, T, E>(&mut self, operation: F) -> Result<T, E>
    where
        F: Fn() -> Fut,
        Fut: Future<Output = Result<T, E>>,
    {
        match self.state {
            CircuitState::Open => {
                if self.last_failure_time.elapsed() > self.cooldown {
                    self.state = CircuitState::HalfOpen;
                } else {
                    return Err(/* circuit open error */);
                }
            }
            CircuitState::HalfOpen | CircuitState::Closed => {}
        }

        match operation().await {
            Ok(result) => {
                self.on_success();
                Ok(result)
            }
            Err(e) => {
                self.on_failure();
                Err(e)
            }
        }
    }

    fn on_success(&mut self) {
        self.failure_count = 0;
        self.state = CircuitState::Closed;
    }

    fn on_failure(&mut self) {
        self.failure_count += 1;
        self.last_failure_time = Instant::now();

        if self.failure_count >= self.failure_threshold {
            self.state = CircuitState::Open;
        }
    }
}
```

## Graceful Degradation

### Service Degradation Levels

#### Level 1: Full Functionality
- All features available
- Normal performance

#### Level 2: Degraded Performance
- All features available
- Slower response times
- Higher latency

#### Level 3: Limited Functionality
- Critical features only
- Non-critical features disabled
- Read-only mode where possible

#### Level 4: Emergency Mode
- Authentication only
- No game sessions
- No publishing
- Emergency announcements only

### Degradation Triggers
- API latency > 2x baseline
- Error rate > 5%
- Database connection pool > 90%
- Memory > 90%
- CPU > 90%

### Degradation Implementation
```rust
pub struct ServiceDegradation {
    level: DegradationLevel,
}

impl ServiceDegradation {
    pub fn check_feature(&self, feature: Feature) -> bool {
        match (self.level, feature) {
            (DegradationLevel::Full, _) => true,
            (DegradationLevel::Performance, _) => true,
            (DegradationLevel::Limited, Feature::Critical) => true,
            (DegradationLevel::Limited, Feature::NonCritical) => false,
            (DegradationLevel::Emergency, Feature::Auth) => true,
            (DegradationLevel::Emergency, _) => false,
        }
    }
}
```

## User Communication

### Error Messages
- **Clear**: Explain what happened
- **Actionable**: Tell user what to do
- **Polite**: Don't blame the user
- **Consistent**: Use consistent terminology

### Loading States
- Show loading indicators for network operations
- Provide estimated time when possible
- Allow cancellation of long operations

### Offline Indicators
- Show network status to user
- Warn when operations will fail offline
- Queue operations for later sync

## Testing Error Handling

### Unit Tests
```rust
#[test]
fn test_retry_on_transient_error() {
    let mut call_count = 0;
    let result = retry_with_backoff(|| async {
        call_count += 1;
        if call_count < 3 {
            Err(Error::Transient)
        } else {
            Ok(())
        }
    }).await;
    assert!(result.is_ok());
    assert_eq!(call_count, 3);
}
```

### Integration Tests
```rust
#[tokio::test]
async fn test_circuit_breaker() {
    let mut breaker = CircuitBreaker::new(3, Duration::from_secs(10));

    // 3 failures should open circuit
    for _ in 0..3 {
        let result = breaker.call(|| async { Err(Error::Transient) }).await;
        assert!(result.is_err());
    }

    // Circuit should be open
    let result = breaker.call(|| async { Ok(()) }).await;
    assert!(result.is_err());
}
```

### Chaos Testing
- Simulate network failures
- Simulate server failures
- Simulate high latency
- Test reconnection logic
- Test state reconciliation

## Implementation Order

1. **Phase 1**: Basic error handling (error codes, error responses)
2. **Phase 2**: Retry logic with exponential backoff
3. **Phase 3**: WebSocket reconnection
4. **Phase 4**: State synchronization and reconciliation
5. **Phase 5**: Circuit breaker pattern
6. **Phase 6**: Graceful degradation
7. **Phase 7**: Offline mode for Studio
8. **Phase 8**: Comprehensive error testing

## Best Practices

### General
- Handle errors at the appropriate layer
- Don't silently swallow errors
- Log all errors with context
- Provide meaningful error messages to users
- Test error paths

### Client-Side
- Always show user feedback
- Allow user to retry failed operations
- Save state locally when possible
- Queue operations for offline sync
- Monitor network state

### Server-Side
- Return consistent error format
- Include request ID in errors
- Log server-side errors
- Implement rate limiting
- Use circuit breakers for external dependencies

## Revisit Triggers
- High error rate despite retries
- Users complain about error messages
- Network issues cause frequent disconnections
- State synchronization issues in games
- New error types discovered

## References
- Circuit Breaker pattern (Martin Fowler)
- Exponential backoff with jitter (AWS blog)
- Resilience patterns (Microsoft Azure)
- WebSocket best practices
