# Configuration Guide

This guide covers all configuration options available in Simple UTC Daemon.

## Configuration File Format

Simple UTC Daemon uses a simple key-value configuration format:

```ini
# Comments start with #
key = value
list_key = ["item1", "item2", "item3"]
boolean_key = true
```

## Configuration File Locations

### Default Locations
- **Linux/macOS**: `/etc/simple-utcd/simple-utcd.conf`
- **Windows**: `C:\ProgramData\Simple-UTCd\simple-utcd.conf`

### Custom Location
```bash
# Specify custom config file
simple-utcd -c /path/to/custom.conf
```

## Configuration Sections

### Network Configuration

#### `listen_address`
- **Type**: String
- **Default**: `0.0.0.0`
- **Description**: IP address to bind the server to
- **Examples**:
  ```ini
  listen_address = 0.0.0.0        # Listen on all interfaces
  listen_address = 127.0.0.1      # Listen only on localhost
  listen_address = 192.168.1.100  # Listen on specific interface
  ```

#### `listen_port`
- **Type**: Integer
- **Default**: `37`
- **Description**: Port number to listen on
- **Examples**:
  ```ini
  listen_port = 37      # Standard UTC port (requires root)
  listen_port = 1037    # Alternative port (non-privileged)
  listen_port = 1234    # Custom port
  ```

#### `enable_ipv6`
- **Type**: Boolean
- **Default**: `true`
- **Description**: Enable IPv6 support
- **Examples**:
  ```ini
  enable_ipv6 = true   # Enable IPv6
  enable_ipv6 = false  # Disable IPv6
  ```

#### `max_connections`
- **Type**: Integer
- **Default**: `1000`
- **Description**: Maximum number of concurrent connections
- **Examples**:
  ```ini
  max_connections = 100    # Low-traffic environment
  max_connections = 1000   # Standard configuration
  max_connections = 10000  # High-traffic environment
  ```

### UTC Server Configuration

#### `stratum`
- **Type**: Integer
- **Default**: `2`
- **Description**: Stratum level of this server
- **Range**: 1-15
- **Examples**:
  ```ini
  stratum = 1    # Primary reference clock
  stratum = 2    # Secondary server
  stratum = 3    # Tertiary server
  ```

#### `sync_interval`
- **Type**: Integer
- **Default**: `64`
- **Description**: Synchronization interval in seconds
- **Examples**:
  ```ini
  sync_interval = 32   # High-frequency sync
  sync_interval = 64   # Standard sync
  sync_interval = 128  # Low-frequency sync
  ```

#### `timeout`
- **Type**: Integer
- **Default**: `1000`
- **Description**: Connection timeout in milliseconds
- **Examples**:
  ```ini
  timeout = 500   # Fast timeout
  timeout = 1000  # Standard timeout
  timeout = 5000  # Slow timeout
  ```

### Logging Configuration

#### `log_file`
- **Type**: String
- **Default**: `/var/log/simple-utcd/simple-utcd.log`
- **Description**: Path to log file
- **Examples**:
  ```ini
  log_file = /var/log/simple-utcd/simple-utcd.log
  log_file = /tmp/simple-utcd.log
  log_file = C:\ProgramData\Simple-UTCd\logs\simple-utcd.log
  ```

#### `log_level`
- **Type**: String
- **Default**: `INFO`
- **Options**: `DEBUG`, `INFO`, `WARN`, `ERROR`, `CRITICAL`
- **Description**: Minimum log level to record
- **Examples**:
  ```ini
  log_level = DEBUG    # Verbose logging
  log_level = INFO     # Standard logging
  log_level = WARN     # Warnings and errors only
  log_level = ERROR    # Errors only
  ```

#### `enable_console_logging`
- **Type**: Boolean
- **Default**: `true`
- **Description**: Enable logging to console
- **Examples**:
  ```ini
  enable_console_logging = true   # Log to console
  enable_console_logging = false  # No console logging
  ```

#### `enable_syslog`
- **Type**: Boolean
- **Default**: `false`
- **Description**: Enable syslog logging
- **Examples**:
  ```ini
  enable_syslog = true   # Use syslog
  enable_syslog = false  # No syslog
  ```

### Security Configuration

#### `enable_authentication`
- **Type**: Boolean
- **Default**: `false`
- **Description**: Enable authentication
- **Examples**:
  ```ini
  enable_authentication = true   # Require authentication
  enable_authentication = false  # No authentication
  ```

#### `authentication_key`
- **Type**: String
- **Default**: `""`
- **Description**: Authentication key for clients
- **Examples**:
  ```ini
  authentication_key = my-secret-key
  authentication_key = 
  ```

#### `allowed_clients`
- **Type**: List of Strings
- **Default**: `[]`
- **Description**: List of allowed client IP addresses or networks
- **Examples**:
  ```ini
  allowed_clients = []
  allowed_clients = ["192.168.1.0/24", "10.0.0.0/8"]
  allowed_clients = ["192.168.1.100", "10.0.0.50"]
  ```

#### `denied_clients`
- **Type**: List of Strings
- **Default**: `[]`
- **Description**: List of denied client IP addresses or networks
- **Examples**:
  ```ini
  denied_clients = []
  denied_clients = ["192.168.1.100", "10.0.0.50"]
  denied_clients = ["192.168.1.0/24"]
  ```

### Performance Configuration

#### `worker_threads`
- **Type**: Integer
- **Default**: `4`
- **Description**: Number of worker threads
- **Examples**:
  ```ini
  worker_threads = 1    # Single-threaded
  worker_threads = 4    # Standard configuration
  worker_threads = 16   # High-performance
  ```

#### `enable_statistics`
- **Type**: Boolean
- **Default**: `true`
- **Description**: Enable statistics collection
- **Examples**:
  ```ini
  enable_statistics = true   # Collect statistics
  enable_statistics = false  # No statistics
  ```

#### `stats_interval`
- **Type**: Integer
- **Default**: `60`
- **Description**: Statistics collection interval in seconds
- **Examples**:
  ```ini
  stats_interval = 30   # Frequent collection
  stats_interval = 60   # Standard collection
  stats_interval = 300  # Infrequent collection
  ```

## Configuration Examples

### Basic Configuration
```ini
# Basic UTC daemon configuration
listen_address = 0.0.0.0
listen_port = 37
enable_ipv6 = true
max_connections = 100

stratum = 2
sync_interval = 64
timeout = 1000

log_level = INFO
enable_console_logging = true
log_file = /var/log/simple-utcd/simple-utcd.log

enable_authentication = false
worker_threads = 2
enable_statistics = true
```

### Secure Configuration
```ini
# Secure UTC daemon configuration
listen_address = 192.168.1.100
listen_port = 37
enable_ipv6 = false
max_connections = 50

stratum = 2
sync_interval = 64
timeout = 1000

log_level = WARN
enable_console_logging = false
enable_syslog = true
log_file = /var/log/simple-utcd/simple-utcd.log

enable_authentication = true
authentication_key = my-secure-key-here
allowed_clients = ["192.168.1.0/24", "10.0.0.0/8"]
denied_clients = ["192.168.1.100"]

worker_threads = 4
enable_statistics = true
stats_interval = 60
```

### High-Performance Configuration
```ini
# High-performance UTC daemon configuration
listen_address = 0.0.0.0
listen_port = 37
enable_ipv6 = true
max_connections = 10000

stratum = 2
sync_interval = 32
timeout = 500

log_level = ERROR
enable_console_logging = false
enable_syslog = true
log_file = /var/log/simple-utcd/simple-utcd.log

enable_authentication = false
worker_threads = 16
enable_statistics = true
stats_interval = 30
```

## Environment Variables

You can override configuration values using environment variables:

```bash
# Override configuration values
export UTCD_LISTEN_ADDRESS=192.168.1.100
export UTCD_LISTEN_PORT=1037
export UTCD_LOG_LEVEL=DEBUG
export UTCD_WORKER_THREADS=8

# Start daemon
simple-utcd -c /etc/simple-utcd/simple-utcd.conf
```

Environment variable format: `UTCD_<CONFIG_KEY>` (uppercase, underscore-separated)

## Configuration Validation

### Test Configuration
```bash
# Test configuration syntax
simple-utcd --config-test -c /etc/simple-utcd/simple-utcd.conf

# Expected output:
# Configuration file is valid
```

### Validate Specific Values
```bash
# Check if port is available
netstat -tulpn | grep :37

# Check if log directory exists
ls -la /var/log/simple-utcd/

# Check if user exists
id simple-utcd
```

## Configuration Reloading

### Signal-Based Reload
```bash
# Reload configuration without restart
sudo kill -HUP $(pgrep simple-utcd)
```

### File Watching (Future Feature)
```ini
# Enable configuration file watching
enable_config_watch = true
config_watch_interval = 5
```

## Best Practices

### Security
- Use authentication in production
- Restrict access with allowed_clients
- Use non-standard ports when possible
- Enable syslog for centralized logging

### Performance
- Set worker_threads based on CPU cores
- Adjust max_connections based on expected load
- Use appropriate log levels
- Enable statistics for monitoring

### Reliability
- Use absolute paths for log files
- Enable log rotation
- Set appropriate timeouts
- Monitor disk space for logs

## Troubleshooting Configuration

### Common Issues

#### Invalid Configuration
```bash
# Check syntax
simple-utcd --config-test -c /etc/simple-utcd/simple-utcd.conf

# Check for typos
grep -n "=" /etc/simple-utcd/simple-utcd.conf
```

#### Permission Issues
```bash
# Check file permissions
ls -la /etc/simple-utcd/simple-utcd.conf

# Fix permissions
sudo chown simple-utcd:simple-utcd /etc/simple-utcd/simple-utcd.conf
sudo chmod 640 /etc/simple-utcd/simple-utcd.conf
```

#### Port Conflicts
```bash
# Check if port is in use
sudo netstat -tulpn | grep :37

# Change port in configuration
listen_port = 1037
```

## Next Steps

- **Deployment**: See [Deployment Guide](../deployment/README.md)
- **Examples**: See [Configuration Examples](../examples/README.md)
- **Troubleshooting**: See [Troubleshooting Guide](../troubleshooting/README.md)

---

*Next: [Deployment Guide](../deployment/README.md) for production deployment strategies*
