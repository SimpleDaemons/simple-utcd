# CLI Reference

This document provides a comprehensive reference for the Simple UTC Daemon command-line interface.

## Command Syntax

```bash
simple-utcd [OPTIONS] [CONFIG_FILE]
```

## Options

### Basic Options

#### `-c, --config <FILE>`
- **Description**: Specify configuration file path
- **Default**: `/etc/simple-utcd/simple-utcd.conf`
- **Example**: `simple-utcd -c /path/to/config.conf`

#### `-d, --daemon`
- **Description**: Run as daemon (background process)
- **Example**: `simple-utcd -d -c /etc/simple-utcd/simple-utcd.conf`

#### `-f, --foreground`
- **Description**: Run in foreground (default)
- **Example**: `simple-utcd -f -c /etc/simple-utcd/simple-utcd.conf`

#### `-h, --help`
- **Description**: Show help message
- **Example**: `simple-utcd --help`

#### `-v, --version`
- **Description**: Show version information
- **Example**: `simple-utcd --version`

### Configuration Options

#### `--config-test`
- **Description**: Test configuration file syntax
- **Example**: `simple-utcd --config-test -c /etc/simple-utcd/simple-utcd.conf`

#### `--config-validate`
- **Description**: Validate configuration and show parsed values
- **Example**: `simple-utcd --config-validate -c /etc/simple-utcd/simple-utcd.conf`

#### `--config-dump`
- **Description**: Dump current configuration to stdout
- **Example**: `simple-utcd --config-dump -c /etc/simple-utcd/simple-utcd.conf`

### Debugging Options

#### `--debug`
- **Description**: Enable debug mode with verbose logging
- **Example**: `simple-utcd --debug -c /etc/simple-utcd/simple-utcd.conf`

#### `--verbose`
- **Description**: Enable verbose output
- **Example**: `simple-utcd --verbose -c /etc/simple-utcd/simple-utcd.conf`

#### `--log-level <LEVEL>`
- **Description**: Set log level (DEBUG, INFO, WARN, ERROR, CRITICAL)
- **Example**: `simple-utcd --log-level DEBUG -c /etc/simple-utcd/simple-utcd.conf`

### Network Options

#### `--listen-address <ADDRESS>`
- **Description**: Override listen address from config
- **Example**: `simple-utcd --listen-address 192.168.1.100 -c /etc/simple-utcd/simple-utcd.conf`

#### `--listen-port <PORT>`
- **Description**: Override listen port from config
- **Example**: `simple-utcd --listen-port 1037 -c /etc/simple-utcd/simple-utcd.conf`

#### `--max-connections <COUNT>`
- **Description**: Override max connections from config
- **Example**: `simple-utcd --max-connections 1000 -c /etc/simple-utcd/simple-utcd.conf`

### Service Management Options

#### `--pid-file <FILE>`
- **Description**: Specify PID file location
- **Example**: `simple-utcd --pid-file /var/run/simple-utcd.pid -c /etc/simple-utcd/simple-utcd.conf`

#### `--user <USER>`
- **Description**: Run as specified user
- **Example**: `simple-utcd --user simple-utcd -c /etc/simple-utcd/simple-utcd.conf`

#### `--group <GROUP>`
- **Description**: Run as specified group
- **Example**: `simple-utcd --group simple-utcd -c /etc/simple-utcd/simple-utcd.conf`

## Command Examples

### Basic Usage
```bash
# Start with default configuration
sudo simple-utcd

# Start with custom configuration
sudo simple-utcd -c /etc/simple-utcd/simple-utcd.conf

# Start as daemon
sudo simple-utcd -d -c /etc/simple-utcd/simple-utcd.conf
```

### Configuration Testing
```bash
# Test configuration syntax
simple-utcd --config-test -c /etc/simple-utcd/simple-utcd.conf

# Validate configuration
simple-utcd --config-validate -c /etc/simple-utcd/simple-utcd.conf

# Dump configuration
simple-utcd --config-dump -c /etc/simple-utcd/simple-utcd.conf
```

### Debugging
```bash
# Run in debug mode
simple-utcd --debug -c /etc/simple-utcd/simple-utcd.conf

# Run with verbose logging
simple-utcd --verbose --log-level DEBUG -c /etc/simple-utcd/simple-utcd.conf

# Run in foreground with debug
simple-utcd -f --debug -c /etc/simple-utcd/simple-utcd.conf
```

### Network Configuration
```bash
# Override network settings
simple-utcd --listen-address 192.168.1.100 --listen-port 1037 -c /etc/simple-utcd/simple-utcd.conf

# Override connection limits
simple-utcd --max-connections 1000 -c /etc/simple-utcd/simple-utcd.conf
```

### Service Management
```bash
# Run as specific user
sudo simple-utcd --user simple-utcd --group simple-utcd -c /etc/simple-utcd/simple-utcd.conf

# Specify PID file
sudo simple-utcd --pid-file /var/run/simple-utcd.pid -c /etc/simple-utcd/simple-utcd.conf
```

## Environment Variables

### Configuration Overrides
```bash
# Override configuration values
export UTCD_LISTEN_ADDRESS=192.168.1.100
export UTCD_LISTEN_PORT=1037
export UTCD_LOG_LEVEL=DEBUG
export UTCD_WORKER_THREADS=8

# Start daemon
simple-utcd -c /etc/simple-utcd/simple-utcd.conf
```

### Available Environment Variables
- `UTCD_LISTEN_ADDRESS`: Override listen address
- `UTCD_LISTEN_PORT`: Override listen port
- `UTCD_LOG_LEVEL`: Override log level
- `UTCD_WORKER_THREADS`: Override worker threads
- `UTCD_MAX_CONNECTIONS`: Override max connections
- `UTCD_LOG_FILE`: Override log file path
- `UTCD_CONFIG_FILE`: Override config file path

## Signal Handling

### Supported Signals
- `SIGTERM`: Graceful shutdown
- `SIGHUP`: Reload configuration
- `SIGUSR1`: Rotate logs
- `SIGUSR2`: Dump statistics

### Signal Usage
```bash
# Graceful shutdown
sudo kill -TERM $(pgrep simple-utcd)

# Reload configuration
sudo kill -HUP $(pgrep simple-utcd)

# Rotate logs
sudo kill -USR1 $(pgrep simple-utcd)

# Dump statistics
sudo kill -USR2 $(pgrep simple-utcd)
```

## Exit Codes

### Exit Code Meanings
- `0`: Success
- `1`: General error
- `2`: Configuration error
- `3`: Network error
- `4`: Permission error
- `5`: Resource error

### Exit Code Examples
```bash
# Check exit code
simple-utcd --config-test -c /etc/simple-utcd/simple-utcd.conf
echo $?

# Handle exit codes in scripts
if simple-utcd --config-test -c /etc/simple-utcd/simple-utcd.conf; then
    echo "Configuration is valid"
else
    echo "Configuration has errors"
    exit 1
fi
```

## Logging Options

### Log Levels
- `DEBUG`: Detailed debug information
- `INFO`: General information
- `WARN`: Warning messages
- `ERROR`: Error messages
- `CRITICAL`: Critical error messages

### Log Output
```bash
# Log to console
simple-utcd --log-level INFO -c /etc/simple-utcd/simple-utcd.conf

# Log to file
simple-utcd --log-file /var/log/simple-utcd/simple-utcd.log -c /etc/simple-utcd/simple-utcd.conf

# Log to syslog
simple-utcd --syslog -c /etc/simple-utcd/simple-utcd.conf
```

## Performance Options

### Worker Threads
```bash
# Set worker threads
simple-utcd --worker-threads 8 -c /etc/simple-utcd/simple-utcd.conf
```

### Connection Limits
```bash
# Set max connections
simple-utcd --max-connections 1000 -c /etc/simple-utcd/simple-utcd.conf
```

### Timeout Settings
```bash
# Set timeout
simple-utcd --timeout 5000 -c /etc/simple-utcd/simple-utcd.conf
```

## Security Options

### Authentication
```bash
# Enable authentication
simple-utcd --enable-authentication --auth-key "secret-key" -c /etc/simple-utcd/simple-utcd.conf
```

### Access Control
```bash
# Set allowed clients
simple-utcd --allowed-clients "192.168.0.0/16,10.0.0.0/8" -c /etc/simple-utcd/simple-utcd.conf

# Set denied clients
simple-utcd --denied-clients "192.168.1.100" -c /etc/simple-utcd/simple-utcd.conf
```

## Monitoring Options

### Statistics
```bash
# Enable statistics
simple-utcd --enable-statistics --stats-interval 60 -c /etc/simple-utcd/simple-utcd.conf
```

### Health Checks
```bash
# Enable health checks
simple-utcd --enable-health-check --health-check-interval 10 -c /etc/simple-utcd/simple-utcd.conf
```

## Troubleshooting Commands

### Configuration Issues
```bash
# Test configuration
simple-utcd --config-test -c /etc/simple-utcd/simple-utcd.conf

# Validate configuration
simple-utcd --config-validate -c /etc/simple-utcd/simple-utcd.conf

# Show parsed configuration
simple-utcd --config-dump -c /etc/simple-utcd/simple-utcd.conf
```

### Network Issues
```bash
# Test network connectivity
simple-utcd --listen-address 127.0.0.1 --listen-port 1037 -c /etc/simple-utcd/simple-utcd.conf

# Test with different port
simple-utcd --listen-port 1037 -c /etc/simple-utcd/simple-utcd.conf
```

### Debugging Issues
```bash
# Run with debug output
simple-utcd --debug --verbose -c /etc/simple-utcd/simple-utcd.conf

# Run with specific log level
simple-utcd --log-level DEBUG -c /etc/simple-utcd/simple-utcd.conf
```

## Scripting Examples

### Startup Script
```bash
#!/bin/bash
# Startup script for simple-utcd

CONFIG_FILE="/etc/simple-utcd/simple-utcd.conf"
PID_FILE="/var/run/simple-utcd.pid"
USER="simple-utcd"
GROUP="simple-utcd"

# Check if configuration exists
if [ ! -f "$CONFIG_FILE" ]; then
    echo "Configuration file not found: $CONFIG_FILE"
    exit 1
fi

# Test configuration
if ! simple-utcd --config-test -c "$CONFIG_FILE"; then
    echo "Configuration test failed"
    exit 1
fi

# Start daemon
simple-utcd -d --pid-file "$PID_FILE" --user "$USER" --group "$GROUP" -c "$CONFIG_FILE"

echo "Simple UTC Daemon started"
```

### Health Check Script
```bash
#!/bin/bash
# Health check script for simple-utcd

HOST="localhost"
PORT="37"
TIMEOUT="5"

# Check if service is running
if ! pgrep -f simple-utcd > /dev/null; then
    echo "❌ Simple UTC Daemon process not running"
    exit 1
fi

# Check if port is listening
if ! nc -z $HOST $PORT; then
    echo "❌ Port $PORT not listening"
    exit 1
fi

# Check if service responds
if ! timeout $TIMEOUT bash -c "echo '' | nc $HOST $PORT" > /dev/null; then
    echo "❌ Service not responding"
    exit 1
fi

echo "✅ Simple UTC Daemon is healthy"
exit 0
```

### Configuration Backup Script
```bash
#!/bin/bash
# Configuration backup script

CONFIG_FILE="/etc/simple-utcd/simple-utcd.conf"
BACKUP_DIR="/var/backups/simple-utcd"
TIMESTAMP=$(date +%Y%m%d_%H%M%S)

# Create backup directory
mkdir -p "$BACKUP_DIR"

# Backup configuration
cp "$CONFIG_FILE" "$BACKUP_DIR/simple-utcd.conf.$TIMESTAMP"

# Keep only last 30 days
find "$BACKUP_DIR" -name "simple-utcd.conf.*" -mtime +30 -delete

echo "Configuration backed up to $BACKUP_DIR/simple-utcd.conf.$TIMESTAMP"
```

## Best Practices

### Command Line Usage
- Always test configuration before starting
- Use absolute paths for configuration files
- Run as non-root user when possible
- Use appropriate log levels
- Monitor exit codes in scripts

### Security
- Use strong authentication keys
- Restrict network access
- Run with minimal privileges
- Monitor access logs
- Keep software updated

### Performance
- Set appropriate worker threads
- Configure connection limits
- Monitor resource usage
- Use appropriate timeouts
- Enable statistics

---

*This CLI reference provides comprehensive information for using the Simple UTC Daemon command-line interface effectively.*
