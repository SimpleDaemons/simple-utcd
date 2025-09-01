# Configuration Examples

This directory contains real-world configuration examples for Simple UTC Daemon in various scenarios.

## Example Categories

### Simple Examples
- **Basic Configuration**: Minimal setup for development and testing
- **Local Development**: Configuration for local development environments
- **Testing**: Configuration optimized for testing scenarios

### Advanced Examples
- **High Performance**: Configuration for high-traffic environments
- **High Security**: Configuration with enhanced security features
- **Load Balancing**: Configuration for load-balanced deployments

### Production Examples
- **Enterprise**: Enterprise-grade configuration with monitoring
- **Cloud**: Configuration optimized for cloud deployments
- **Multi-Site**: Configuration for multi-site deployments

## Quick Start Examples

### Basic Development Setup
```ini
# config/examples/simple/development.conf
listen_address = 127.0.0.1
listen_port = 1037
log_level = DEBUG
enable_console_logging = true
enable_authentication = false
worker_threads = 1
max_connections = 10
```

### Production Setup
```ini
# config/examples/production/enterprise.conf
listen_address = 0.0.0.0
listen_port = 37
log_level = WARN
enable_console_logging = false
enable_syslog = true
enable_authentication = true
authentication_key = ${UTCD_AUTH_KEY}
allowed_clients = ["192.168.0.0/16", "10.0.0.0/8"]
worker_threads = 8
max_connections = 10000
enable_statistics = true
```

## Example Files

### Simple Examples
- `simple-utcd.conf.example` - Basic configuration template
- `development.conf` - Development environment setup
- `testing.conf` - Testing environment configuration

### Advanced Examples
- `high-performance.conf` - High-traffic configuration
- `high-security.conf` - Security-focused configuration
- `load-balanced.conf` - Load balancer configuration

### Production Examples
- `enterprise.conf` - Enterprise deployment
- `cloud.conf` - Cloud-optimized configuration
- `multi-site.conf` - Multi-site deployment

## Usage Instructions

### Using Examples
1. **Copy Example**: Copy the appropriate example file
2. **Customize**: Modify settings for your environment
3. **Test**: Validate configuration before deployment
4. **Deploy**: Use the configuration in production

### Example Commands
```bash
# Copy simple example
cp config/examples/simple/simple-utcd.conf.example /etc/simple-utcd/simple-utcd.conf

# Copy production example
cp config/examples/production/enterprise.conf /etc/simple-utcd/simple-utcd.conf

# Test configuration
simple-utcd --config-test -c /etc/simple-utcd/simple-utcd.conf
```

## Configuration Templates

### Environment Variables
Many examples use environment variables for sensitive data:
```bash
# Set environment variables
export UTCD_AUTH_KEY="your-secret-key"
export UTCD_LOG_LEVEL="INFO"
export UTCD_WORKER_THREADS="4"

# Start with environment variables
simple-utcd -c /etc/simple-utcd/simple-utcd.conf
```

### Template Variables
Configuration templates support variable substitution:
```ini
# Template variables
listen_address = ${UTCD_LISTEN_ADDRESS:-0.0.0.0}
listen_port = ${UTCD_LISTEN_PORT:-37}
log_level = ${UTCD_LOG_LEVEL:-INFO}
```

## Best Practices

### Security
- Use strong authentication keys
- Restrict network access
- Enable logging and monitoring
- Use environment variables for secrets

### Performance
- Set appropriate worker threads
- Configure connection limits
- Enable statistics collection
- Monitor resource usage

### Reliability
- Use absolute paths
- Enable log rotation
- Configure health checks
- Set up monitoring

## Customization Guide

### Network Configuration
```ini
# Basic network settings
listen_address = 0.0.0.0
listen_port = 37
enable_ipv6 = true
max_connections = 1000
```

### Security Configuration
```ini
# Security settings
enable_authentication = true
authentication_key = your-secret-key
allowed_clients = ["192.168.0.0/16"]
denied_clients = ["192.168.1.100"]
```

### Performance Configuration
```ini
# Performance settings
worker_threads = 8
timeout = 1000
sync_interval = 64
enable_statistics = true
```

### Logging Configuration
```ini
# Logging settings
log_level = INFO
log_file = /var/log/simple-utcd/simple-utcd.log
enable_console_logging = false
enable_syslog = true
```

## Troubleshooting Examples

### Common Issues
- **Permission Denied**: Use non-privileged ports or run as root
- **Port Conflicts**: Change port or kill conflicting processes
- **Configuration Errors**: Validate configuration syntax
- **Network Issues**: Check firewall and network connectivity

### Debug Configuration
```ini
# Debug configuration
log_level = DEBUG
enable_console_logging = true
enable_authentication = false
worker_threads = 1
max_connections = 10
```

## Next Steps

- **Deployment**: See [Deployment Examples](deployment.md)
- **Configuration**: See [Configuration Guide](../configuration/README.md)
- **Troubleshooting**: See [Troubleshooting Guide](../troubleshooting/README.md)

---

*These examples provide a starting point for configuring Simple UTC Daemon in various environments.*
