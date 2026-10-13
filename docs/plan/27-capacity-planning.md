# Capacity Planning and Validation

## Context
Target: 700 concurrent users (CCU) with current provider commitments:
- Huggingface CPU: 2 instances × (2 VCPUs, 16GB RAM)
- PostgreSQL: 8GB total (3GB Aiven + 5GB Layerbase)
- Valkey: 3GB total
- Cloudflare Workers: 100k req/day for Studio APIs

This document defines the capacity model, measurements needed, and validation strategy.

## Resource Breakdown

### Huggingface CPU Resources
**Total available**: 2 instances × 2 VCPUs × 16GB RAM = 4 VCPUs, 32GB RAM

**Allocation**:
- Instance 1 (Main Backend): 2 VCPUs, 16GB
  - Auth/session APIs
  - Economy/ledger operations
  - Publishing/version management
  - Moderation APIs
  - Fallback for Cloudflare Workers

- Instance 2 (Game Handler/Authority): 2 VCPUs, 16GB
  - WebSocket connections for game sessions
  - Game request validation
  - Session coordination
  - Real-time state synchronization

### Database Resources
**PostgreSQL**: 8GB total storage
- Connection pool sizing needed
- Query performance under load
- Index strategy for hot paths

**Valkey**: 3GB total
- Session storage
- Rate limiting
- Cache for hot data
- Temporary game state

## Capacity Model

### Per-User Resource Estimates (Initial Assumptions - MUST MEASURE)

**Game Session (Game Handler)**:
- WebSocket connection: ~4KB memory overhead
- Session state: ~1KB per user (position, basic state)
- Network: ~1KB/sec for real-time updates (varies by game)
- CPU: ~0.01 CPU sec/sec per user (varies by game complexity)

**Backend API (Main Backend)**:
- Auth API: ~0.001 CPU sec per request
- Session refresh: ~0.0005 CPU sec per request
- Economy operations: ~0.005 CPU sec per transaction
- Database queries: ~0.002 CPU sec per query

**Database**:
- Session table: ~200 bytes per row
- User table: ~500 bytes per row
- Economy ledger: ~300 bytes per transaction
- Index overhead: ~2-3x data size

**Valkey**:
- Session cache: ~500 bytes per session
- Rate limit counters: ~100 bytes per user
- Hot cache entries: ~1KB per entry

### Theoretical Capacity Calculations

**Game Handler (2 VCPUs, 16GB RAM)**:
- Memory-limited: 16GB / (4KB + 1KB) = ~3,200 users per instance
- CPU-limited: 2 VCPUs / 0.01 CPU = ~200 users per CPU (400 total)
- **Bottleneck**: CPU, theoretical max ~400 CCU per instance

**Main Backend (2 VCPUs, 16GB RAM)**:
- API traffic varies by user behavior
- Assume 10 API calls per user per minute
- 700 users × 10 calls/min = 7,000 calls/min = 117 calls/sec
- CPU required: 117 × 0.002 CPU sec = 0.234 CPU sec/sec = 23% CPU utilization
- **Status**: Comfortable headroom

**PostgreSQL (8GB)**:
- 700 users × 200 bytes (session) = 140KB
- 700 users × 500 bytes (user) = 350KB
- Assume 10 transactions per user per day = 7,000 × 300 bytes = 2.1MB
- **Status**: Storage not a concern, connection pool and query performance are

**Valkey (3GB)**:
- 700 sessions × 500 bytes = 350KB
- Rate limit counters × 100 bytes = 70KB
- **Status**: Memory not a concern, eviction policy needed

### Critical Bottlenecks to Validate

1. **WebSocket connection handling**: Can 2 VCPUs handle 700 concurrent WebSockets?
2. **Database connection pool**: How many concurrent DB connections needed?
3. **Network bandwidth**: What's the actual throughput per game session?
4. **Game handler CPU**: Actual CPU usage per game session (varies by game complexity)

## Measurement Strategy

### Phase 1: Baseline Measurements (Single User)
- Create a single game session
- Measure:
  - Memory per WebSocket connection
  - CPU usage per second
  - Network bytes/sec
  - Database queries per minute
  - Valkey operations per minute

### Phase 2: Scale Testing (Gradual Load)
- Test with 10, 50, 100, 200, 400, 700 concurrent users
- Measure at each tier:
  - CPU utilization (p50, p95, p99)
  - Memory usage
  - API latency (p50, p95, p99)
  - Database query latency
  - WebSocket message latency
  - Error rates

### Phase 3: Stress Testing (Beyond Target)
- Test up to 1,000 CCU to find breaking point
- Identify first bottleneck
- Document degradation behavior

### Phase 4: Long-Running Stability
- 24-hour test at 500 CCU
- Monitor for memory leaks
- Monitor connection pool exhaustion
- Monitor database bloat

## Tools and Metrics

### CPU Metrics
- System CPU utilization (user, system, idle)
- Per-process CPU usage
- CPU time per request
- Context switches

### Memory Metrics
- RSS memory per process
- Heap allocations (if applicable)
- Connection pool memory
- Buffer pool size

### Network Metrics
- Bytes in/out per second
- Packet rate
- Connection count
- Latency (ping time)

### Database Metrics
- Connection pool utilization
- Query latency (p50, p95, p99)
- Query rate per second
- Deadlocks and lock contention
- Disk I/O

### Valkey Metrics
- Memory usage
- Hit/miss ratio
- Operations per second
- Eviction rate

### Application Metrics
- Requests per second
- Error rate (4xx, 5xx)
- API latency per endpoint
- WebSocket message latency
- Session creation/destruction rate

## Validation Criteria

### Must Pass for 700 CCU
- CPU utilization < 80% p95 on both instances
- Memory usage < 80% of available RAM
- API latency p95 < 200ms for critical endpoints
- WebSocket message latency p95 < 100ms
- Database query latency p95 < 50ms
- Error rate < 0.1%
- No memory leaks over 24 hours
- No connection pool exhaustion

### Warning Signs
- CPU > 60% sustained at 500 CCU
- Memory growing over time (leak)
- Database query latency increasing with load
- WebSocket connection failures
- High GC pauses (if applicable)

### Failure Modes
- CPU > 90% at target load → Need more CPU
- Memory > 90% at target load → Need more RAM
- Database connection pool exhausted → Need connection pooling optimization
- Network saturation → Need bandwidth optimization
- Latency degradation → Need query optimization or caching

## Capacity Scaling Strategy

### If 700 CCU Not Achievable
**Option 1: Optimize**
- Profile hot paths
- Optimize database queries
- Add caching for hot data
- Reduce state per session

**Option 2: Scale Up**
- Request additional Huggingface CPU instances
- Add more database capacity
- Implement database sharding

**Option 3: Reduce Target**
- Lower initial CCU target
- Implement session queuing
- Add rate limiting

### When to Scale
- Add capacity when p95 CPU > 70% at target load
- Add capacity when p95 latency exceeds SLA
- Add capacity when error rate > 1%

## Game-Specific Capacity Considerations

### Variable Factors
Different games have different resource requirements:
- Static games (chess, turn-based): Low CPU, low network
- Action games (shooter, platformer): High CPU, high network
- Physics-heavy games: Very high CPU
- Chat-heavy games: High network, moderate CPU

### Capacity by Game Type
Create separate capacity models for:
- Light games: Target 1,000 CCU
- Medium games: Target 500 CCU
- Heavy games: Target 200 CCU

### Game Classification
- Require creators to classify game complexity
- Set different limits per game type
- Monitor actual usage vs classification

## Monitoring Dashboard

### Real-Time Metrics
- Current CCU
- CPU utilization per instance
- Memory usage per instance
- API latency per endpoint
- Database latency
- WebSocket message rate
- Error rate

### Historical Metrics
- CCU over time
- Peak CCU per day
- Average session duration
- API traffic patterns
- Database query patterns

### Alerts
- CPU > 80% for 5 minutes
- Memory > 80% for 5 minutes
- API latency p95 > 500ms
- Error rate > 1%
- Database connection pool > 90%
- Valkey memory > 90%

## Next Steps

1. **Build minimal game handler** with single WebSocket connection
2. **Measure baseline** for single user
3. **Build load test tool** to simulate multiple users
4. **Run scale tests** incrementally (10 → 50 → 100 → 200 → 400 → 700)
5. **Document actual measurements** vs theoretical estimates
6. **Adjust capacity model** based on real data
7. **Implement monitoring** before production launch
8. **Set up alerts** based on validation criteria

## Revisit Triggers
- Actual measurements differ from estimates by >50%
- New game types with different resource requirements
- Provider changes (Huggingface CPU limits, database performance)
- Architecture changes (new services, different deployment)

## References
- Huggingface Spaces CPU documentation
- PostgreSQL performance tuning
- Valkey optimization guide
- Rust profiling tools (flamegraph, perf)
- WebSocket performance best practices
