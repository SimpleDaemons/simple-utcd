# Simple UTC Daemon Documentation

Welcome to the Simple UTC Daemon documentation. This guide will help you understand, configure, and deploy the Simple UTC Daemon effectively.

## Product Versions

There is one build, licensed under Apache-2.0. Enterprise and datacenter editions are not part of this project.

### 🏭 Production Version
**License:** Apache 2.0
**Status:** Version 1.0.0
**Target:** Hosts that need RFC 868 time (TCP and UDP port 37)

- RFC 868 timestamps (seconds since 1900-01-01 UTC), from the host clock
- TCP and UDP listeners, IPv4 and optional IPv6
- Client allow/deny lists, including IPv4 CIDR, and per-client rate limits
- Multi-format configuration (JSON, YAML, INI)
- Hot reload configuration
- Cross-platform support

**Documentation:** [Production Version Documentation](production/README.md)

## Documentation Structure

### 📚 Shared Documentation
Common documentation applicable to all versions:

- **[Getting Started](shared/getting-started/)** - Installation and quick start guides
- **[Configuration](shared/configuration/)** - Configuration reference and examples
- **[Diagrams](shared/diagrams/)** - Architecture and flow diagrams
- **[Troubleshooting](shared/troubleshooting/)** - Common issues and debugging
- **[User Guide](shared/user-guide/)** - User documentation
- **[Examples](shared/examples/)** - Usage examples
- **[Recovery](shared/recovery/)** - Disaster recovery procedures

### 🏭 Production Version Documentation
- **[Production Guide](production/README.md)** - Complete Production Version documentation
- **[Installation](production/installation.md)** - Production installation guide
- **[Configuration](production/configuration.md)** - Production configuration reference
- **[Deployment](production/deployment.md)** - Production deployment guide
- **[Security](production/security.md)** - Production security best practices
- **[Performance](production/performance.md)** - Production performance tuning
- **[Operations](production/operations.md)** - Production operations guide

### 👨‍💻 Developer Documentation
Documentation for developers and contributors:

- **[Developer Guide](development/README.md)** - Complete developer documentation
- **[Build Guide](development/BUILD_GUIDE.md)** - Build commands and reference
- **[Setup Guide](development/SETUP.md)** - Development environment setup

---

## Quick Start

### Production Version
1. [Install Simple UTC Daemon](shared/getting-started/installation.md)
2. [Quick Start Guide](shared/getting-started/quick-start.md)
3. [Production Configuration](production/configuration.md)
4. [Production Deployment](production/deployment.md)

## Documentation by Topic

### Getting Started
- [Installation Guide](shared/getting-started/installation.md) - Install on Linux, macOS, Windows
- [Quick Start](shared/getting-started/quick-start.md) - Get running in minutes
- [First Steps](shared/getting-started/first-steps.md) - Basic configuration

### Configuration
- [Configuration Reference](shared/configuration/README.md) - Complete configuration guide
- [Production Configuration](production/configuration.md) - Production-specific configuration
- [Enterprise Configuration](enterprise/configuration.md) - Enterprise-specific configuration
- [Datacenter Configuration](datacenter/configuration.md) - Datacenter-specific configuration

### Deployment
- [Production Deployment](production/deployment.md) - Production deployment guide
- [Enterprise Deployment](enterprise/deployment.md) - Enterprise deployment guide
- [Datacenter Deployment](datacenter/deployment.md) - Datacenter deployment guide
- [Docker Deployment](shared/deployment/docker.md) - Containerized deployment
- [High Availability](enterprise/high-availability.md) - HA setup (Enterprise+)

### Operations
- [Production Operations](production/operations.md) - Production operations guide
- [Monitoring](shared/deployment/monitoring.md) - Monitoring setup
- [Backup Procedures](shared/deployment/backup-procedures.md) - Backup and restore
- [Maintenance](shared/deployment/maintenance-procedures.md) - Maintenance procedures

### Security
- [Production Security](production/security.md) - Production security best practices
- [Enterprise Security](enterprise/security.md) - Advanced security features
- [Security Best Practices](shared/user-guide/security-best-practices.md) - General security guide

### Performance
- [Production Performance](production/performance.md) - Production performance tuning
- [Enterprise Performance](enterprise/performance.md) - Enterprise performance optimization
- [Datacenter Performance](datacenter/performance.md) - Datacenter scaling and optimization

### Troubleshooting
- [Common Issues](shared/troubleshooting/README.md) - Troubleshooting guide
- [Debugging](shared/troubleshooting/debugging.md) - Debugging techniques
- [Performance Issues](shared/troubleshooting/performance.md) - Performance troubleshooting

---

## Contributing to Documentation

We welcome contributions to improve this documentation. Please see our [Contributing Guide](../CONTRIBUTING.md) for details on how to contribute.

## Feedback

If you find any issues with the documentation or have suggestions for improvement, please:

1. Open an issue on [GitHub](https://github.com/SimpleDaemons/simple-utcd/issues)
2. Submit a pull request with your improvements
3. Contact us at docs@simpledaemons.com

---

**Last Updated:** December 2024
**Production Version:** 0.3.2
**Enterprise Version:** Planned
**Datacenter Version:** Planned
