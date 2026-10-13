# Monitoring and Observability Strategy

## Context
Poko has multiple services that need monitoring:
- Rust backend (Huggingface CPU)
- Rust game handler (Huggingface CPU)
- Cloudflare Workers (Studio APIs)
- PostgreSQL (Aiven + Layerbase)
- Valkey (Aiven)
- Need comprehensive observability to detect issues early

## Monitoring Stack

### Tool Selection
**Metrics**: Prometheus + Grafana (industry standard, open source)
**Logs**: Structured JSON logs + Loki (for log aggregation)
**Tracing**: OpenTelemetry + Jaeger (for distributed tracing)
**Alerting**: Alertmanager (for alert routing)
**Dashboards**: Grafana (for visualization)

### Deployment
Since all services are on external providers:
- **Self-hosted monitoring**: Run monitoring stack on one Huggingface CPU instance
- **Or**: Use provider-specific monitoring (Huggingface metrics, Cloudflare analytics)
- **Recommended**: Hybrid - use provider metrics + minimal self-hosted dashboards

## Metrics Strategy

### Metrics Categories

#### 1. System Metrics
**Rust Backend/Game Handler**:
- CPU utilization (user, system, idle)
- Memory usage (RSS, heap)
- Network bytes in/out
- Disk I/O (if applicable)
- File descriptors
- Thread count

**PostgreSQL**:
- Connection pool utilization
- Query latency (p50, p95, p99)
- Query rate per second
- Deadlocks
- Lock wait time
- Disk I/O
- Cache hit ratio

**Valkey**:
- Memory usage
- Hit/miss ratio
- Operations per second
- Eviction rate
- Connection count
- Blocked clients

**Cloudflare Workers**:
- Request count
- Error rate (4xx, 5xx)
- Latency (p50, p95, p99)
- CPU time
- Memory usage
- Edge cache hit ratio

#### 2. Application Metrics
**Rust Backend**:
- Requests per second (per endpoint)
- Request latency (p50, p95, p99 per endpoint)
- Error rate (per endpoint, per error type)
- Session creation/destruction rate
- Failed authentication attempts
- Rate limit violations
- Database query count
- Cache hit/miss ratio

**Rust Game Handler**:
- Concurrent WebSocket connections
- Connection creation/destruction rate
- Message rate (messages/sec)
- Message latency (p50, p95, p99)
- Session state size
- Validation failures
- Network bytes per session

**Cloudflare Workers**:
- Requests per second (per endpoint)
- Error rate (per endpoint)
- Latency (p50, p95, p99)
- Cache hit ratio
- WASM execution time

#### 3. Business Metrics
- Concurrent users (CCU)
- Active sessions
- Games in progress
- API calls per user
- Session duration (p50, p95, p99)
- New user registrations
- Published experiences
- Asset uploads

### Metrics Collection

**Rust Services**:
```rust
// Use prometheus-client crate
use prometheus::{Counter, Histogram, Gauge, Registry};

lazy_static! {
    static ref REGISTRY: Registry = Registry::new();
    static ref REQUEST_COUNT: Counter = Counter::new("requests_total", "Total requests").unwrap();
    static ref REQUEST_LATENCY: Histogram = Histogram::new("request_duration_seconds", "Request latency").unwrap();
    static ref ACTIVE_CONNECTIONS: Gauge = Gauge::new("active_connections", "Active WebSocket connections").unwrap();
}

// Expose metrics endpoint
async fn metrics() -> String {
    REGISTRY.gather()
}
```

**PostgreSQL**:
- Use pg_stat_statements extension
- Expose metrics via PostgreSQL exporter
- Or query pg_stat_* tables directly

**Valkey**:
- Use Valkey exporter (redis_exporter)
- Or INFO command

**Cloudflare Workers**:
- Built-in Analytics Dashboard
- Or export metrics to external service

### Metrics Storage
- **Short-term**: In-memory (1-2 hours)
- **Medium-term**: Prometheus (7-30 days)
- **Long-term**: External service (Datadog, New Relic) if needed

## Logging Strategy

### Log Format
Structured JSON logs for machine parsing:
```json
{
  "timestamp": "2024-01-15T10:30:00Z",
  "level": "info",
  "service": "backend",
  "request_id": "abc123",
  "user_id": "user456",
  "endpoint": "/api/auth/login",
  "method": "POST",
  "status_code": 200,
  "duration_ms": 45,
  "message": "User authenticated successfully"
}
```

### Log Levels
- **ERROR**: Errors that require attention
- **WARN**: Warning conditions (deprecated usage, near capacity)
- **INFO**: Normal operational information
- **DEBUG**: Detailed diagnostic information
- **TRACE**: Very detailed tracing (disabled in production)

### Log Content
**Must include**:
- Timestamp
- Service name
- Log level
- Request ID (correlation ID)
- User ID (if authenticated)
- Error details (if error)

**Should include**:
- Endpoint/route
- HTTP method
- Status code
- Duration
- IP address
- User agent

### Log Collection
**Rust Services**:
```rust
// Use tracing crate
use tracing::{info, error, instrument};

#[instrument(skip(password))]
async fn login(username: &str, password: &str) -> Result<()> {
    info!(username, "Login attempt");
    // ...
}
```

**Cloudflare Workers**:
```typescript
// Workers has built-in logging
console.log({ timestamp: Date.now(), level: 'info', message: '...' });
```

### Log Storage
- **Short-term**: In-memory (buffer)
- **Medium-term**: Loki or provider logs (7-30 days)
- **Long-term**: External log service if needed
- **Cost optimization**: Rotate logs, compress old logs

## Tracing Strategy

### Distributed Tracing
Use OpenTelemetry for end-to-end request tracing:
- Trace ID: Correlates logs across services
- Span ID: Individual operation within a trace
- Parent span ID: Relationship between operations

### Trace Flow
```
Client Request → Backend API → Database Query → Cache Lookup → Response
              ↘ Workers API ↗
```

### Tracing Implementation
**Rust Services**:
```rust
use opentelemetry::trace::Tracer;
use tracing_opentelemetry::OpenTelemetryLayer;

#[instrument]
async fn handle_request(request: Request) -> Response {
    // Automatic tracing
}
```

**Cloudflare Workers**:
```typescript
// Workers supports OpenTelemetry via bindings
// Or manual tracing with trace headers
```

### Trace Storage
- **Short-term**: In-memory (1-2 hours)
- **Medium-term**: Jaeger (7 days)
- **Long-term**: External tracing service if needed

## Alerting Strategy

### Alert Severity Levels
- **P0 (Critical)**: System down, data loss, security breach
- **P1 (High)**: Major functionality broken, high error rate
- **P2 (Medium)**: Degraded performance, partial outage
- **P3 (Low)**: Warning, near capacity, non-critical issues

### Alert Rules

#### P0 Alerts (Immediate Response)
- **Service down**: All backend instances unreachable (5 minutes)
- **Database down**: PostgreSQL unreachable (1 minute)
- **Cache down**: Valkey unreachable (5 minutes)
- **Security breach**: Unauthorized access detected (immediate)
- **Data loss**: Database corruption or data deletion (immediate)

#### P1 Alerts (Response within 15 minutes)
- **High error rate**: Error rate > 5% for 5 minutes
- **High latency**: API latency p95 > 1s for 5 minutes
- **Connection pool exhausted**: DB connections > 90% for 5 minutes
- **Memory pressure**: Memory > 90% for 5 minutes
- **Disk space**: Disk > 90% (if applicable)

#### P2 Alerts (Response within 1 hour)
- **Elevated error rate**: Error rate > 1% for 15 minutes
- **Elevated latency**: API latency p95 > 500ms for 15 minutes
- **High CPU**: CPU > 80% for 15 minutes
- **High memory**: Memory > 80% for 15 minutes
- **Rate limit violations**: > 100/minute for 5 minutes

#### P3 Alerts (Response within 4 hours)
- **Warning error rate**: Error rate > 0.1% for 30 minutes
- **Warning latency**: API latency p95 > 200ms for 30 minutes
- **Moderate CPU**: CPU > 60% for 30 minutes
- **Moderate memory**: Memory > 60% for 30 minutes
- **Slow queries**: DB query p95 > 100ms for 30 minutes

### Alert Routing
- **P0**: PagerDuty/Phone call to on-call engineer
- **P1**: Slack message + SMS to on-call engineer
- **P2**: Slack message to team
- **P3**: Email to team (or Slack channel)

### Alert Suppression
- Suppress alerts during planned maintenance
- Suppress alerts during deployment windows
- Aggregate similar alerts (deduplication)
- Cool-down period (don't alert every minute)

## Dashboard Strategy

### Real-Time Dashboards

#### System Overview
- CPU utilization per instance
- Memory usage per instance
- Network traffic
- Error rate
- Request rate

#### Backend Dashboard
- Requests per second (per endpoint)
- Latency (p50, p95, p99 per endpoint)
- Error rate (per endpoint)
- Database query latency
- Cache hit ratio
- Session creation/destruction rate

#### Game Handler Dashboard
- Concurrent WebSocket connections
- Message rate
- Message latency
- Validation failures
- Network bytes per session
- Active sessions

#### Database Dashboard
- Connection pool utilization
- Query latency (p50, p95, p99)
- Query rate
- Deadlocks
- Lock wait time
- Cache hit ratio

#### Cloudflare Workers Dashboard
- Request count
- Error rate
- Latency
- CPU time
- Edge cache hit ratio

### Historical Dashboards

#### Daily Summary
- Daily active users
- Daily API calls
- Daily error rate
- Daily average latency
- Daily peak CCU

#### Weekly Summary
- Weekly active users
- Weekly API calls
- Weekly error rate
- Weekly average latency
- Weekly peak CCU

#### Monthly Summary
- Monthly active users
- Monthly API calls
- Monthly error rate
- Monthly average latency
- Monthly peak CCU

## Performance Monitoring

### Key Performance Indicators (KPIs)
- **Uptime**: 99.9% monthly (43 minutes downtime allowed)
- **API Latency**: p95 < 200ms for critical endpoints
- **Error Rate**: < 0.1% for all endpoints
- **Session Success Rate**: > 99.5%
- **WebSocket Message Latency**: p95 < 100ms

### Performance Budgets
- **Auth API**: < 100ms p95
- **Session API**: < 50ms p95
- **Economy API**: < 200ms p95
- **Game Handler Validation**: < 10ms p95
- **Database Query**: < 50ms p95
- **Cache Lookup**: < 5ms p95

### Performance Regression Detection
- Compare current performance to baseline
- Alert on > 20% degradation
- Investigate regressions before deployment
- Use load testing to validate performance

## Security Monitoring

### Security Metrics
- Failed authentication attempts
- Rate limit violations
- Suspicious IP addresses
- Unusual API patterns
- Privilege escalation attempts
- Session hijacking attempts

### Security Alerts
- Brute force attack (100+ failed logins from same IP)
- DDoS attack (10x normal traffic)
- SQL injection attempts
- XSS attempts
- Unauthorized API access

### Audit Logging
- Log all admin actions
- Log all privileged operations
- Log all authentication events
- Log all access to sensitive data
- Keep audit logs for 90 days

## Implementation Order

1. **Phase 1**: Basic logging (structured JSON logs)
2. **Phase 2**: Metrics collection (system + application metrics)
3. **Phase 3**: Alerting (P0 + P1 alerts)
4. **Phase 4**: Dashboards (real-time overview)
5. **Phase 5**: Distributed tracing
6. **Phase 6**: Historical dashboards
7. **Phase 7**: Security monitoring
8. **Phase 8**: Performance regression detection

## Best Practices

### General
- Monitor everything, alert on what matters
- Use correlation IDs for request tracing
- Keep alert noise low (alert fatigue)
- Review and adjust alerts regularly
- Document alert runbooks

### Metrics
- Use appropriate metric types (counter, gauge, histogram)
- Label metrics consistently
- Avoid high cardinality labels
- Sample high-frequency metrics
- Set up metric retention policies

### Logs
- Structure logs for machine parsing
- Include correlation IDs
- Don't log sensitive data
- Set log retention policies
- Monitor log volume

### Alerts
- Alert on symptoms, not causes
- Set clear severity levels
- Define response times per severity
- Create runbooks for common alerts
- Test alert routing regularly

## Revisit Triggers
- Alert fatigue (too many false positives)
- Monitoring becomes bottleneck
- New services added
- Security incident reveals gaps
- Performance issues not detected

## References
- Prometheus best practices
- OpenTelemetry documentation
- Grafana dashboard examples
- Alertmanager configuration
- Structured logging best practices
