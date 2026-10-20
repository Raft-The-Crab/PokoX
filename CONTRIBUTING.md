# Contributing to PokoX

Thank you for your interest in contributing to PokoX. As this is a proprietary project, contributions are accepted only from authorized personnel under specific agreements.

## Contribution Eligibility

Contributions are accepted from:
- PokoX employees and contractors
- Authorized partners with signed contribution agreements
- Approved third-party contributors under explicit written agreement

If you are not an authorized contributor, please do not submit pull requests or issue patches.

## For Authorized Contributors

### Getting Started

1. Read the [AGENTS.md](AGENTS.md) protocol for coding agents
2. Review the [TASKS.md](TASKS.md) ledger to understand current priorities
3. Check [STATUS.md](STATUS.md) for the current milestone and repository state
4. Read the relevant contract under `docs/` for the area you're working on

### Development Workflow

1. **Select a task**: Work on the first `READY` task in TASKS.md
2. **Read contracts**: Understand the requirements and acceptance criteria
3. **Implement**: Write production-ready code with tests
4. **Verify**: Build and test your changes
5. **Document**: Update relevant contracts and task status
6. **Submit**: Create a pull request with clear description

### Code Standards

- **Language**: C++20 unless specified otherwise in an ADR
- **Build System**: CMake
- **Style**: Follow `.clang-format` configuration
- **Documentation**: Public C++ APIs require Doxygen comments
- **Testing**: Add tests with implementation, not later

### Security Requirements

- Never commit secrets, credentials, or private keys
- Treat all external input as untrusted
- Validate inputs at public boundaries
- Follow security checklist in `docs/12-security/`

### Submitting Changes

1. Ensure all tests pass
2. Run linting and formatting tools
3. Update relevant documentation
4. Update TASKS.md and STATUS.md with verification evidence
5. Reference the task ID in your pull request (e.g., "Implements PKX-001")

### Pull Request Guidelines

- Keep PRs focused on a single task
- Include clear description of what changed and why
- Link to relevant contracts and ADRs
- Attach build/test results
- Request review from appropriate maintainers

## Issue Reporting

Authorized contributors can report issues through the project's issue tracker. Please include:

- Clear description of the problem
- Steps to reproduce
- Expected vs actual behavior
- Environment details (OS, toolchain versions)
- Relevant logs or error messages

## Code of Conduct

All contributors must adhere to the [Code of Conduct](CODE_OF_CONDUCT.md).

## Questions

For questions about contribution eligibility or development practices, contact the PokoX engineering team.

---

**Note**: Unauthorized contributions will not be accepted. If you're interested in contributing to PokoX, please reach out to partnerships@pokox.com to discuss potential collaboration.
