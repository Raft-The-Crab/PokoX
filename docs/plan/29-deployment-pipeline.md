# Deployment Pipeline Strategy

## Context
Poko has multiple deployment targets:
- Huggingface CPU (Rust backend)
- Huggingface CPU (Rust game handler)
- Cloudflare Workers (TypeScript + Rust WASM)
- Need automated, reliable deployment process

## Deployment Architecture

### Environments
- **Development**: Local development with Docker
- **Staging**: Huggingface Space + Cloudflare Workers (staging)
- **Production**: Huggingface Space + Cloudflare Workers (production)

### Deployment Flow
```
Git Push → CI Pipeline → Build → Test → Deploy Staging → Manual Approval → Deploy Production
```

## CI/CD Pipeline

### Tool Selection
- **CI Provider**: GitHub Actions (free for public repos, sufficient for private)
- **Container Registry**: GitHub Container Registry (ghcr.io)
- **Deployment**: Custom scripts + provider-specific CLIs

### Pipeline Stages

#### Stage 1: Build
```yaml
- Trigger: Push to main or pull request
- Steps:
  1. Checkout code
  2. Install Rust toolchain
  3. Install Node.js (for Workers)
  4. Build Rust backend (release mode)
  5. Build Rust game handler (release mode)
  6. Build Cloudflare Workers (TypeScript + Rust WASM)
  7. Build Docker images (if using containers)
  8. Upload artifacts
```

#### Stage 2: Test
```yaml
- Steps:
  1. Run unit tests (Rust)
  2. Run integration tests (Rust)
  3. Run Workers tests (TypeScript)
  4. Run security audit (cargo audit, npm audit)
  5. Run linters (clippy, eslint)
  6. Check formatting (rustfmt, prettier)
  7. Generate test coverage report
```

#### Stage 3: Deploy Staging
```yaml
- Trigger: Merge to main branch
- Steps:
  1. Build deployment artifacts
  2. Deploy Rust backend to Huggingface staging
  3. Deploy Rust game handler to Huggingface staging
  4. Deploy Cloudflare Workers to staging environment
  5. Run smoke tests against staging
  6. Notify team of staging deployment
```

#### Stage 4: Manual Approval
```yaml
- Trigger: Manual action in GitHub UI
- Steps:
  1. Prompt for approval
  2. Show staging test results
  3. Show diff from previous deployment
  4. Require approval from authorized team member
```

#### Stage 5: Deploy Production
```yaml
- Trigger: Manual approval
- Steps:
  1. Deploy Rust backend to Huggingface production
  2. Deploy Rust game handler to Huggingface production
  3. Deploy Cloudflare Workers to production
  4. Run smoke tests against production
  5. Update database migrations (if needed)
  6. Monitor for errors (first 5 minutes)
  7. Notify team of production deployment
```

## Huggingface CPU Deployment

### Deployment Method
Huggingface Spaces uses Git-based deployment. Two approaches:

#### Option 1: Git-Based Deployment (Recommended)
```bash
# Huggingface Space is a Git repository
# Deploy by pushing to Huggingface remote
git remote add huggingface https://huggingface.co/spaces/your-org/poko-backend
git push huggingface main
```

**Pros**:
- Simple, native to Huggingface
- Automatic builds on push
- Built-in versioning

**Cons**:
- No separate build artifacts
- Must push source code

#### Option 2: Container-Based Deployment
```bash
# Build Docker image locally
docker build -t ghcr.io/your-org/poko-backend:latest .
docker push ghcr.io/your-org/poko-backend:latest

# Huggingface Space pulls image
# Configure Space to use container
```

**Pros**:
- Separate build artifacts
- Can use private registry
- Better control over build environment

**Cons**:
- More complex setup
- Must manage container registry

**Recommendation**: Option 1 (Git-based) for simplicity

### Huggingface Space Configuration

**Backend Space**:
```yaml
# README.md (Huggingface Space config)
---
title: Poko Backend
emoji: 🎮
colorFrom: blue
colorTo: purple
sdk: docker
pinned: false
license: mit
---
```

**Dockerfile**:
```dockerfile
FROM rust:1.75 as builder
WORKDIR /app
COPY . .
RUN cargo build --release

FROM debian:bookworm-slim
RUN apt-get update && apt-get install -y ca-certificates
COPY --from=builder /app/target/release/poko-backend /usr/local/bin/
EXPOSE 8080
CMD ["poko-backend"]
```

**Deployment Script**:
```bash
#!/bin/bash
# deploy-backend.sh

set -e

echo "Building backend..."
cargo build --release

echo "Deploying to Huggingface..."
git add .
git commit -m "Deploy backend: $(date)"
git push huggingface main

echo "Deployment complete"
```

## Cloudflare Workers Deployment

### Deployment Method
Use Wrangler CLI (official Cloudflare Workers tool)

**Installation**:
```bash
npm install -g wrangler
```

**Configuration** (wrangler.toml):
```toml
name = "poko-studio"
main = "src/index.ts"
compatibility_date = "2024-01-01"

[env.staging]
name = "poko-studio-staging"

[env.production]
name = "poko-studio"
vars = { ENVIRONMENT = "production" }
```

**Rust WASM Integration**:
```typescript
// src/index.ts
import init, { handle_request } from '../poko-wasm/pkg/poko_wasm.js';

export default {
  async fetch(request, env, ctx) {
    await init();
    return handle_request(request, env);
  }
};
```

**Deployment Script**:
```bash
#!/bin/bash
# deploy-workers.sh

set -e

echo "Building Rust WASM..."
wasm-pack build --target web --out-dir pkg

echo "Building Workers..."
npm run build

echo "Deploying to staging..."
wrangler deploy --env staging

echo "Deploying to production (requires approval)..."
# wrangler deploy # Run separately with approval
```

## Database Migrations

### Migration Tool
Use `sqlx-cli` for Rust PostgreSQL migrations

**Installation**:
```bash
cargo install sqlx-cli
```

**Migration Structure**:
```
migrations/
  000001_initial_schema.down.sql
  000001_initial_schema.up.sql
  000002_add_sessions_table.down.sql
  000002_add_sessions_table.up.sql
```

**Migration Workflow**:
```bash
# Create new migration
sqlx migrate add add_new_table

# Run migrations (development)
sqlx migrate run

# Run migrations (staging)
DATABASE_URL=$STAGING_DB_URL sqlx migrate run

# Run migrations (production)
DATABASE_URL=$PRODUCTION_DB_URL sqlx migrate run

# Rollback migration
sqlx migrate revert
```

**Automated Migrations in CI**:
```yaml
- Stage: Deploy Staging
  - Run migrations against staging database
  - Rollback if migration fails

- Stage: Deploy Production
  - Run migrations against production database
  - Rollback if migration fails
  - Monitor for errors
```

## Rollback Strategy

### Types of Rollbacks

#### 1. Code Rollback
```bash
# Revert to previous commit
git revert HEAD
git push huggingface main
wrangler deploy
```

#### 2. Database Rollback
```bash
# Revert last migration
sqlx migrate revert
```

#### 3. Full Rollback (Code + Database)
```bash
# 1. Revert code
git revert HEAD
git push huggingface main
wrangler deploy

# 2. Revert database
sqlx migrate revert
```

### Rollback Triggers
- Automated tests fail in staging
- Smoke tests fail in production
- Error rate > 5% in production
- Latency > 2x baseline in production
- Manual rollback request

### Rollback Time Target
- Goal: < 5 minutes from detection to rollback complete
- Practice rollbacks regularly

## Staging Environment

### Purpose
- Test deployment process
- Run integration tests
- Manual QA before production
- Performance testing

### Configuration
- Separate Huggingface Space
- Separate Cloudflare Workers environment
- Separate database (can use Layerbase 5GB)
- Separate Valkey instance (can use Aiven)

### Data Strategy
- Use anonymized/synthetic data
- Or copy production data with PII removed
- Clear staging data regularly

## Secrets Management

### Strategy
Never commit secrets to Git. Use environment variables.

**Development**:
```bash
# .env (gitignored)
DATABASE_URL=postgresql://...
REDIS_URL=redis://...
JWT_SECRET=...
```

**CI/CD**:
```yaml
# GitHub Actions secrets
DATABASE_URL: ${{ secrets.DATABASE_URL }}
REDIS_URL: ${{ secrets.REDIS_URL }}
JWT_SECRET: ${{ secrets.JWT_SECRET }}
```

**Huggingface**:
```bash
# Space Settings → Variables
DATABASE_URL=postgresql://...
REDIS_URL=redis://...
JWT_SECRET=...
```

**Cloudflare Workers**:
```bash
# wrangler.toml or wrangler secret
wrangler secret put JWT_SECRET
```

## Monitoring Deployment

### Deployment Metrics
- Deployment frequency
- Deployment duration
- Deployment success rate
- Rollback frequency
- Time to rollback

### Post-Deployment Monitoring
- Error rate (first 5, 15, 60 minutes)
- Latency (p50, p95, p99)
- Throughput
- Database connection pool
- Memory usage
- CPU usage

### Automated Health Checks
```yaml
# After deployment
- Run health check endpoint
- Run smoke tests
- Monitor metrics for 5 minutes
- Alert on failures
```

## Blue-Green Deployment (Future)

### Current State
- Simple deployment (replace current version)
- Potential downtime during deployment

### Future Improvement
- Deploy new version alongside old
- Switch traffic to new version
- Rollback instant (switch back to old)
- Zero downtime deployment

### Implementation
- Load balancer between versions
- Health checks per version
- Gradual traffic shift (10% → 50% → 100%)

## Implementation Order

1. **Phase 1**: Local development setup (Docker, local DB)
2. **Phase 2**: GitHub Actions CI pipeline (build + test)
3. **Phase 3**: Huggingface deployment script
4. **Phase 4**: Cloudflare Workers deployment script
5. **Phase 5**: Database migration automation
6. **Phase 6**: Staging environment setup
7. **Phase 7**: Production deployment with approval
8. **Phase 8**: Rollback automation
9. **Phase 9**: Monitoring and alerting
10. **Phase 10**: Blue-green deployment (future)

## Best Practices

### Git Workflow
- Use feature branches
- Pull requests for review
- Main branch for deployments
- Semantic versioning

### Testing
- Run tests before every deployment
- Run smoke tests after deployment
- Test rollback process regularly

### Documentation
- Document deployment process
- Document rollback process
- Document common issues
- Update on every change

### Security
- Never commit secrets
- Use least privilege for deployment credentials
- Audit deployment access
- Rotate credentials regularly

## Revisit Triggers
- Deployment process becomes bottleneck
- Rollback time > 5 minutes
- Frequent deployment failures
- New deployment targets added
- Team size increases

## References
- GitHub Actions documentation
- Huggingface Spaces documentation
- Cloudflare Workers documentation
- Wrangler CLI documentation
- sqlx migration documentation
