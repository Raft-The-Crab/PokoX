# Performance Budgets and Service Level Agreements

## Context
Poko needs concrete performance targets to:
- Guide optimization efforts
- Set expectations for users
- Detect performance regressions
- Measure success objectively

## Service Level Objectives (SLOs)

### Overall Platform SLOs
- **Availability**: 99.9% monthly (43 minutes downtime allowed)
- **Uptime during peak hours**: 99.95% (22 minutes downtime allowed per month during peak)
- **Successful request rate**: 99.5% of all requests succeed
- **Data durability**: 99.999% (5 nines, < 0.001% data loss)

### Per-Service SLOs

#### Rust Backend (Huggingface CPU)
- **Availability**: 99.9% monthly
- **API latency p95**: < 200ms for critical endpoints
- **API latency p99**: < 500ms for critical endpoints
- **Error rate**: < 0.5% for all endpoints
- **Successful auth rate**: > 99.9%

#### Rust Game Handler (Huggingface CPU)
- **Availability**: 99.95% monthly (higher priority for real-time)
- **WebSocket connection success rate**: > 99%
- **Message latency p95**: < 100ms
- **Message latency p99**: < 200ms
- **Reconnection success rate**: > 95%
- **Error rate**: < 1%

#### Cloudflare Workers (Studio APIs)
- **Availability**: 99.5% monthly (lower priority, fallback to Huggingface)
- **API latency p95**: < 300ms
- **Error rate**: < 1%
- **Fallback trigger rate**: < 5% (when routed to Huggingface)

#### PostgreSQL (Aiven + Layerbase)
- **Availability**: 99.95% monthly
- **Query latency p95**: < 50ms
- **Query latency p99**: < 100ms
- **Connection pool utilization**: < 80%
- **Deadlock rate**: < 0.01%

#### Valkey (Aiven)
- **Availability**: 99.9% monthly
- **Operation latency p95**: < 5ms
- **Operation latency p99**: < 10ms
- **Hit ratio**: > 80%
- **Memory utilization**: < 80%

## Performance Budgets

### API Endpoints

#### Authentication Endpoints
| Endpoint | Budget (p95) | Budget (p99) | Priority |
|----------|--------------|--------------|----------|
| POST /auth/login | 100ms | 200ms | Critical |
| POST /auth/refresh | 50ms | 100ms | Critical |
| POST /auth/logout | 50ms | 100ms | Critical |
| GET /auth/sessions | 100ms | 200ms | High |
| DELETE /auth/sessions/{id} | 50ms | 100ms | High |

#### User Endpoints
| Endpoint | Budget (p95) | Budget (p99) | Priority |
|----------|--------------|--------------|----------|
| GET /users/me | 50ms | 100ms | Critical |
| PATCH /users/me | 100ms | 200ms | High |
| GET /users/{id} | 100ms | 200ms | Medium |

#### Session Endpoints
| Endpoint | Budget (p95) | Budget (p99) | Priority |
|----------|--------------|--------------|----------|
| POST /sessions | 100ms | 200ms | Critical |
| GET /sessions/{id} | 50ms | 100ms | Critical |
| DELETE /sessions/{id} | 50ms | 100ms | Critical |

#### Economy Endpoints
| Endpoint | Budget (p95) | Budget (p99) | Priority |
|----------|--------------|--------------|----------|
| GET /economy/balance | 50ms | 100ms | Critical |
| POST /economy/transaction | 200ms | 500ms | Critical |
| GET /economy/history | 200ms | 500ms | Medium |

#### Publishing Endpoints
| Endpoint | Budget (p95) | Budget (p99) | Priority |
|----------|--------------|--------------|----------|
| POST /publish | 5s | 10s | High (async) |
| GET /publish/status/{id} | 100ms | 200ms | High |
| GET /experiences/{id} | 100ms | 200ms | High |

#### Studio APIs (Cloudflare Workers)
| Endpoint | Budget (p95) | Budget (p99) | Priority |
|----------|--------------|--------------|----------|
| GET /studio/assets | 200ms | 500ms | Medium |
| GET /studio/metadata | 100ms | 200ms | Medium |
| POST /studio/upload | 2s | 5s | Low (async) |

### Game Handler Performance

#### WebSocket Operations
| Operation | Budget (p95) | Budget (p99) | Priority |
|-----------|--------------|--------------|----------|
| Connection establishment | 500ms | 1s | Critical |
| Message processing | 10ms | 20ms | Critical |
| State validation | 5ms | 10ms | Critical |
| State broadcast | 50ms | 100ms | High |

#### Per-Session Budgets
| Metric | Budget | Priority |
|--------|--------|----------|
| Memory per session | 5KB | Critical |
| Network per session | 2KB/s | High |
| CPU per session | 0.01 CPU | High |
| State updates per second | 10 | Medium |

### Engine Performance

#### Frame Time Budgets
| Platform | Target (60 FPS) | Target (30 FPS) | Priority |
|----------|-----------------|-----------------|----------|
| Windows (mid-tier) | 16.6ms | 33.3ms | Critical |
| Windows (low-tier) | 16.6ms | 33.3ms | Critical |
| Android (high-tier) | 16.6ms | 33.3ms | Critical |
| Android (mid-tier) | 16.6ms | 33.3ms | Critical |
| Android (low-tier) | 33.3ms | 66.6ms | High |

#### Subsystem Budgets
| Subsystem | Budget (60 FPS) | Budget (30 FPS) | Priority |
|-----------|----------------|----------------|----------|
| Physics simulation | 5ms | 10ms | Critical |
| Rendering | 8ms | 16ms | Critical |
| Script execution | 2ms | 4ms | High |
| Audio processing | 1ms | 2ms | Medium |
| UI rendering | 2ms | 4ms | Medium |

## Resource Budgets

### Memory Budgets

#### PokoX Client
| Platform | Budget | Priority |
|----------|--------|----------|
| Windows (minimum) | 512MB RAM | Critical |
| Windows (recommended) | 2GB RAM | High |
| Android (minimum) | 2GB RAM | Critical |
| Android (recommended) | 4GB RAM | High |

#### Engine Runtime
| Component | Budget | Priority |
|-----------|--------|----------|
| Base engine | 100MB | Critical |
| Physics world | 50MB | High |
| Render targets | 200MB | High |
| Audio buffers | 50MB | Medium |
| Script VM | 20MB | Medium |
| Total | 500MB | Critical |

#### Backend Services
| Service | Budget | Priority |
|---------|--------|----------|
| Rust backend (RSS) | 2GB | Critical |
| Rust game handler (RSS) | 2GB | Critical |
| PostgreSQL connections | 100MB | High |
| Valkey connections | 50MB | Medium |

### Network Budgets

#### Client Bandwidth
| Activity | Budget | Priority |
|----------|--------|----------|
| Initial download | 100MB | Critical |
| Asset download per session | 50MB | High |
| Real-time game data | 2KB/s | Critical |
| Voice chat (if added) | 20KB/s | Medium |

#### Server Bandwidth
| Metric | Budget (700 CCU) | Priority |
|--------|------------------|----------|
| Total outbound | 1.4GB/s (2KB/s × 700) | Critical |
| Total inbound | 700MB/s (1KB/s × 700) | Critical |
| Peak burst | 5× sustained | High |

## Latency Targets

### Geographic Latency Targets
| Region | Target (APAC-first) | Priority |
|--------|-------------------|----------|
| Singapore | < 50ms | Critical |
| Japan | < 100ms | High |
| Korea | < 100ms | High |
| Australia | < 150ms | Medium |
| Indonesia | < 100ms | High |
| Philippines | < 100ms | High |

### Latency Budget Breakdown
| Operation | Budget | Priority |
|-----------|--------|----------|
| Client to edge | 50ms | Critical |
| Edge to backend | 50ms | Critical |
| Backend processing | 50ms | Critical |
| Database query | 20ms | Critical |
| Cache lookup | 5ms | Critical |
| Total critical path | 175ms | Critical |

## Throughput Targets

### API Throughput
| Metric | Target | Priority |
|--------|--------|----------|
| Requests per second (total) | 10,000 | High |
| Requests per second (per user) | 0.1 | Medium |
| Concurrent sessions | 700 | Critical |
| Peak burst capacity | 2× target | High |

### Database Throughput
| Metric | Target | Priority |
|--------|--------|----------|
| Queries per second | 5,000 | High |
| Transactions per second | 100 | Critical |
| Concurrent connections | 50 | Critical |

### Cache Throughput
| Metric | Target | Priority |
|--------|--------|----------|
| Operations per second | 20,000 | High |
| Hit ratio | > 80% | Critical |
| Eviction rate | < 1% | Medium |

## Performance Monitoring

### Key Performance Indicators (KPIs)

#### Real-Time KPIs
- Current CCU
- API latency (p50, p95, p99) per endpoint
- Error rate per endpoint
- WebSocket message latency
- Database query latency
- Cache hit ratio
- CPU utilization per instance
- Memory utilization per instance

#### Historical KPIs
- Daily CCU peak
- Daily average latency
- Daily error rate
- Monthly uptime
- Monthly successful request rate
- Monthly data loss incidents

### Performance Regression Detection

#### Automated Detection
- Compare current metrics to 7-day baseline
- Alert on > 20% degradation in latency
- Alert on > 2× increase in error rate
- Alert on > 20% decrease in hit ratio
- Alert on > 20% increase in resource usage

#### Manual Review
- Weekly performance review
- Monthly trend analysis
- Quarterly capacity planning
- Annual performance budget adjustment

### Performance Budget Enforcement

#### Pre-Deployment Checks
- Run performance tests for every PR
- Block deployment if budget exceeded
- Require performance regression tests
- Document performance impact

#### Post-Deployment Monitoring
- Monitor metrics for 1 hour after deployment
- Automated rollback if budget exceeded
- Document performance changes
- Update baseline if legitimate change

## Performance Testing

### Load Testing Strategy

#### Unit Performance Tests
- Test individual functions for performance
- Benchmark critical algorithms
- Measure memory allocations
- Test with realistic data sizes

#### Integration Performance Tests
- Test API endpoints with load
- Test database queries with concurrent load
- Test cache performance
- Test WebSocket message throughput

#### End-to-End Performance Tests
- Simulate 700 CCU
- Measure end-to-end latency
- Measure resource utilization
- Test under stress (1000+ CCU)

### Performance Test Tools
- **Rust**: Criterion (benchmarking), criterion-rs
- **Load testing**: k6, locust
- **Database**: pgbench
- **Cache**: redis-benchmark
- **Network**: iperf

### Performance Test Scenarios

#### Scenario 1: Idle Users
- 700 connected users
- Minimal activity
- Measure baseline resource usage

#### Scenario 2: Active API Users
- 700 users
- 10 API calls per minute per user
- Measure API latency and throughput

#### Scenario 3: Active Game Sessions
- 700 users in game sessions
- 10 state updates per second per session
- Measure WebSocket latency and throughput

#### Scenario 4: Mixed Load
- 350 idle users
- 250 active API users
- 100 active game sessions
- Measure combined resource usage

#### Scenario 5: Stress Test
- 1000+ concurrent users
- Peak activity
- Find breaking point

## Performance Optimization Strategy

### Optimization Priorities

#### P0 (Critical)
- Anything exceeding critical budget
- Database queries > 50ms p95
- API latency > 200ms p95 for critical endpoints
- WebSocket message latency > 100ms p95
- Memory leaks

#### P1 (High)
- Anything exceeding high budget
- Database queries > 100ms p95
- API latency > 500ms p95
- Cache hit ratio < 80%
- CPU > 80%

#### P2 (Medium)
- Anything exceeding medium budget
- Latency degradation < 20%
- Resource usage < 20% above budget
- Optimization opportunities

### Optimization Techniques

#### Database Optimization
- Add indexes for hot queries
- Optimize query plans
- Use prepared statements
- Implement query caching
- Use read replicas (if needed)

#### Cache Optimization
- Increase cache size
- Improve cache hit ratio
- Implement cache warming
- Use multi-level caching
- Optimize cache key design

#### Code Optimization
- Profile hot paths
- Reduce allocations
- Use more efficient algorithms
- Parallelize where appropriate
- Use async/await efficiently

#### Network Optimization
- Compress responses
- Use HTTP/2 or HTTP/3
- Implement CDN caching
- Optimize payload size
- Use binary protocols where appropriate

## Performance Budget Violation Handling

### Immediate Actions
- Alert on critical violations
- Investigate root cause
- Implement temporary mitigation
- Rollback if recent deployment

### Short-Term Actions (within 24 hours)
- Profile and optimize hot paths
- Add caching
- Scale resources if needed
- Implement rate limiting

### Long-Term Actions (within 30 days)
- Architectural changes if needed
- Database schema optimization
- Service decomposition
- Load balancing improvements

## Performance Reporting

### Daily Report
- CCU peak and average
- API latency p95 per endpoint
- Error rate per endpoint
- Resource utilization
- Budget violations

### Weekly Report
- Weekly CCU trend
- Weekly latency trend
- Weekly error rate trend
- Performance improvements made
- Budget violations and resolutions

### Monthly Report
- Monthly uptime vs SLO
- Monthly error budget vs SLO
- Monthly latency budget vs SLO
- Performance improvement summary
- Budget adjustment recommendations

## Performance Budget Review

### Review Triggers
- Monthly scheduled review
- After major feature release
- After architecture change
- After capacity change
- When budget consistently exceeded

### Review Outcomes
- Adjust budgets if legitimate change
- Add new budgets for new features
- Remove obsolete budgets
- Update optimization priorities
- Document rationale for changes

## Implementation Order

1. **Phase 1**: Define initial budgets and SLOs
2. **Phase 2**: Implement performance monitoring
3. **Phase 3**: Add performance regression tests
4. **Phase 4**: Implement automated alerts
5. **Phase 5**: Run baseline performance tests
6. **Phase 6**: Optimize critical paths
7. **Phase 7**: Implement performance dashboards
8. **Phase 8**: Regular performance reviews

## Best Practices

### General
- Measure before optimizing
- Optimize for p95, not average
- Document performance decisions
- Share performance data with team
- Review budgets regularly

### Testing
- Test with realistic data
- Test at target scale
- Test under stress
- Measure all the time
- Automate performance tests

### Monitoring
- Monitor everything important
- Alert on budget violations
- Investigate regressions quickly
- Document findings
- Update budgets based on data

## Revisit Triggers
- Performance budget consistently exceeded
- New performance requirements
- Architecture changes
- User complaints about performance
- Capacity changes

## References
- Google SRE book (SLOs, SLIs, error budgets)
- AWS Well-Architected Framework (performance efficiency)
- Microsoft Azure Performance Pillar
- Web Performance Working Group
