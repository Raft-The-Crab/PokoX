# Security Policy

## Supported Versions

Security updates are currently only provided for the active development branch. Production releases will have supported version windows defined in release notes.

## Reporting a Vulnerability

If you discover a security vulnerability in PokoX, please report it responsibly.

### How to Report

**Email**: security@pokox.com

Please include:
- Description of the vulnerability
- Steps to reproduce (if applicable)
- Potential impact assessment
- Your contact information for follow-up

### What to Expect

- We will acknowledge receipt of your report within 48 hours
- We will provide a detailed response within 7 days
- We will work with you to understand and verify the issue
- We will coordinate disclosure and patch release timeline
- We will credit you in the release notes (if you wish)

### Please Do Not

- Publicly disclose the vulnerability before coordinated disclosure
- Exploit the vulnerability against production systems
- Share details with unauthorized parties

## Security Features

PokoX includes multiple security features by design:

- **Runtime sandboxing** for user scripts (MUTE)
- **Input validation** at all public boundaries
- **Memory safety** practices in C++ code
- **Anti-cheat architecture** for multiplayer scenarios
- **Asset validation** to prevent malicious content
- **Network security** for online services

See [docs/12-security/](docs/12-security/) for detailed security architecture and threat model.

## Dependency Security

All third-party dependencies are reviewed for:
- Active maintenance and security history
- Compatible license
- Known vulnerabilities
- Security best practices

Dependencies are pinned to specific versions to prevent supply chain attacks.

## Security Audits

Periodic security audits will be conducted on:
- Core engine components
- Network protocols
- Scripting runtime
- Authentication and authorization systems
- Input handling and asset processing

Audit results will be summarized in release notes.

## Best Practices for Contributors

- Never commit secrets, credentials, or private keys
- Treat all external input as untrusted
- Validate and sanitize user inputs
- Follow the security checklist in `docs/12-engineering/security-checklist.md`
- Report potential security issues through the proper channels

## Contact

For security-related questions not requiring immediate vulnerability disclosure, contact security@pokox.com.
