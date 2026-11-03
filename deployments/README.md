# Deployment Configurations

Deployment manifests and configurations for PokoX services.

## Contents

- **docker**: Docker container definitions and Dockerfiles
- **helm**: Helm charts for Kubernetes deployments (future)

## Environments

- Local development
- CI/CD
- Staging (non-production data)
- Closed alpha
- Production (only after all gates pass)

## Deployment Artifacts

- Go backend container/image with immutable build identifier
- C++ authority runtime image/binary with protocol compatibility ID
- PokoX client and Studio artifacts for Windows/Android
- Poko Mod and Review Host artifacts
- Database migration bundle
- Package signing public-key set and protected private-key reference
- Software bill of materials and dependency/license inventory

## Policy

Build once and promote the same artifact between environments.
Never rebuild from a floating dependency set for production.
