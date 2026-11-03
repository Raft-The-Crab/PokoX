# ADR-0008 — Two-Account Hugging Face Deployment Is Conditional

**Status:** Accepted with provider eligibility gate  
**Date:** 2026-11-03

## Context
The desired deployment separates the Go platform backend and C++ game authority into two Hugging Face accounts.

## Decision
Retain Account A = Go backend and Account B = C++ authority as a deployment topology, but do not classify it as free until both accounts independently have eligible Docker compute access. Current official Spaces documentation says Docker/compute-backed Gradio requires PRO for personal accounts or Team/Enterprise for organizations; the free personal ZeroGPU exception is not general-purpose Docker hosting. Free Spaces sleep after inactivity and disk is non-persistent. Authority transport must be prototyped.

## Consequences
- Two free accounts do not bypass the per-account compute restriction.
- Account separation remains useful for secrets/failure domains when both are eligible.
- Local development and community hosting remain fallbacks until a reliable compute host is available.
