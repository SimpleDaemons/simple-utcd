# User Guide

This guide provides comprehensive information for users of Simple UTC Daemon.

## Overview

Simple UTC Daemon is a lightweight, high-performance implementation of the UTC (Universal Time Coordinate) protocol as defined in RFC 868. It provides accurate time synchronization services for network clients.

## Key Features

- **RFC 868 Compliance**: Full implementation of the UTC protocol
- **Cross-Platform**: Windows, macOS, and Linux support
- **High Performance**: Multi-threaded architecture with connection pooling
- **Security**: Authentication and access control
- **Monitoring**: Built-in statistics and health checks
- **Configurable**: Flexible configuration options

## Understanding UTC Protocol

### What is UTC?
UTC (Universal Time Coordinate) is a time standard that provides a consistent time reference across different time zones. It's based on Coordinated Universal Time and is used as the standard for time synchronization.

### How UTC Works
1. **Client Request**: Client connects to UTC server on port 37
2. **Time Query**: Client sends a request for current time
3. **Server Response**: Server responds with 4-byte timestamp
4. **Time Calculation**: Client converts timestamp to local time

### UTC vs NTP
- **UTC**: Simple protocol, single request/response
- **NTP**: Complex protocol with multiple exchanges
- **Use Case**: UTC for simple time queries, NTP for continuous synchronization

## Basic Usage

### Starting the Service
```bash
# Start with default configuration
sudo simple-utcd

# Start with custom configuration
sudo simple-utcd -c /path/to/config.conf

# Start in foreground (for testing)
sudo simple-utcd -c /etc/simple-utcd/simple-utcd.conf --foreground
```

### Stopping the Service
```bash
# Stop service
sudo systemctl stop simple-utcd

# Or send SIGTERM
sudo kill -TERM $(pgrep simple-utcd)
```

### Service Management
```bash
# Check status
sudo systemctl status simple-utcd

# Enable auto-start
sudo systemctl enable simple-utcd

# Disable auto-start
sudo systemctl disable simple-utcd

# Restart service
sudo systemctl restart simple-utcd

# Reload configuration
sudo systemctl reload simple-utcd
```

## Configuration Management

### Configuration File Location
- **Linux/macOS**: `/etc/simple-utcd/simple-utcd.conf`
- **Windows**: `C:\ProgramData\Simple-UTCd\simple-utcd.conf`

### Basic Configuration
```ini
# Network settings
listen_address = 0.0.0.0
listen_port = 37

# Security settings
enable_authentication = false
max_connections = 100

# Logging settings
log_level = INFO
log_file = /var/log/simple-utcd/simple-utcd.log
```

### Configuration Validation
```bash
# Test configuration syntax
simple-utcd --config-test -c /etc/simple-utcd/simple-utcd.conf

# Validate and show parsed configuration
simple-utcd --config-validate -c /etc/simple-utcd/simple-utcd.conf
```

## Client Usage

### Python Client Example
```python
import socket
import struct
import time
from datetime import datetime

def get_utc_time(host='localhost', port=37):
    """Get UTC time from server"""
    try:
        # Connect to UTC server
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(5)
        sock.connect((host, port))

        # Receive 4-byte timestamp
        data = sock.recv(4)
        sock.close()

        if len(data) != 4:
            raise ValueError("Invalid response length")

        # Convert from UTC epoch (1900) to Unix epoch (1970)
        utc_timestamp = struct.unpack('!I', data)[0]
        unix_timestamp = utc_timestamp - 2208988800

        return datetime.fromtimestamp(unix_timestamp, tz=time.timezone.utc)

    except Exception as e:
        print(f"Error getting UTC time: {e}")
        return None

# Usage
utc_time = get_utc_time('192.168.1.100', 37)
if utc_time:
    print(f"UTC Time: {utc_time}")
```

### C Client Example
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

int get_utc_time(const char* host, int port) {
    int sockfd;
    struct sockaddr_in server_addr;
    uint32_t utc_timestamp;
    time_t unix_timestamp;

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return -1;
    }

    // Set up server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, host, &server_addr.sin_addr);

    // Connect to server
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sockfd);
        return -1;
    }

    // Receive timestamp
    if (recv(sockfd, &utc_timestamp, sizeof(utc_timestamp), 0) != sizeof(utc_timestamp)) {
        perror("recv");
        close(sockfd);
        return -1;
    }

    close(sockfd);

    // Convert from UTC epoch (1900) to Unix epoch (1970)
    unix_timestamp = ntohl(utc_timestamp) - 2208988800;

    printf("UTC Time: %s", ctime(&unix_timestamp));
    return 0;
}

int main() {
    return get_utc_time("localhost", 37);
}
```

### Command Line Client
```bash
#!/bin/bash
# Simple UTC client script

get_utc_time() {
    local host=${1:-localhost}
    local port=${2:-37}

    # Connect and get timestamp
    local timestamp=$(echo "" | nc $host $port | od -An -td4 | tr -d ' ')

    if [ -n "$timestamp" ]; then
        # Convert from UTC epoch (1900) to Unix epoch (1970)
        local unix_timestamp=$((timestamp - 2208988800))

        # Convert to readable format
        date -d "@$unix_timestamp" "+%Y-%m-%d %H:%M:%S UTC"
    else
        echo "Failed to get UTC time"
        return 1
    fi
}

# Usage
get_utc_time "192.168.1.100" 37
```

## Monitoring and Statistics

### Built-in Statistics
```bash
# Check service statistics
curl http://localhost:8080/stats

# Check health status
curl http://localhost:8080/health
```

### Log Monitoring
```bash
# Monitor logs in real-time
tail -f /var/log/simple-utcd/simple-utcd.log

# Filter for specific events
grep "ERROR" /var/log/simple-utcd/simple-utcd.log
grep "New connection" /var/log/simple-utcd/simple-utcd.log
```

### Performance Monitoring
```bash
# Check connection count
netstat -an | grep :37 | grep ESTABLISHED | wc -l

# Check memory usage
ps aux | grep simple-utcd

# Check CPU usage
top -p $(pgrep simple-utcd)
```

## Security Considerations

### Authentication
```ini
# Enable authentication
enable_authentication = true
authentication_key = your-secret-key
```

### Access Control
```ini
# Allow specific networks
allowed_clients = ["192.168.0.0/16", "10.0.0.0/8"]

# Deny specific hosts
denied_clients = ["192.168.1.100"]
```

### Firewall Configuration
```bash
# Allow UTC traffic
sudo ufw allow from 192.168.0.0/16 to any port 37
sudo ufw allow from 10.0.0.0/8 to any port 37

# Deny all other traffic
sudo ufw deny 37
```

## Performance Tuning

### System Tuning
```bash
# Increase file descriptor limits
echo "simple-utcd soft nofile 65536" >> /etc/security/limits.conf
echo "simple-utcd hard nofile 65536" >> /etc/security/limits.conf

# Optimize network settings
echo "net.core.rmem_max = 134217728" >> /etc/sysctl.conf
echo "net.core.wmem_max = 134217728" >> /etc/sysctl.conf
sysctl -p
```

### Application Tuning
```ini
# Increase worker threads
worker_threads = 8

# Increase connection limit
max_connections = 1000

# Optimize timeouts
timeout = 500
```

## Troubleshooting

### Common Issues
1. **Service won't start**: Check configuration and permissions
2. **No response**: Check firewall and network connectivity
3. **High CPU usage**: Reduce worker threads or check for loops
4. **Memory issues**: Check for memory leaks or reduce connection limits

### Debug Mode
```bash
# Run in debug mode
simple-utcd -c /etc/simple-utcd/simple-utcd.conf --debug

# Enable verbose logging
log_level = DEBUG
```

### Health Checks
```bash
# Basic health check
nc -zv localhost 37

# Comprehensive health check
curl http://localhost:8080/health
```

## Best Practices

### Configuration
- Use absolute paths for log files
- Enable log rotation
- Set appropriate timeouts
- Configure access control
- Use authentication in production

### Monitoring
- Monitor system resources
- Set up log monitoring
- Configure alerts
- Regular health checks
- Performance monitoring

### Security
- Use strong authentication keys
- Restrict network access
- Keep software updated
- Monitor access logs
- Use firewall rules

### Maintenance
- Regular configuration backups
- Monitor disk space
- Update software regularly
- Test configuration changes
- Document changes

## Integration Examples

### System Integration
```bash
# Add to system startup
sudo systemctl enable simple-utcd

# Add to cron for regular checks
*/5 * * * * /usr/local/bin/check-utc-health.sh
```

### Application Integration
```python
# Django settings
UTC_SERVER = '192.168.1.100'
UTC_PORT = 37

# Flask application
from flask import Flask
import utc_client

app = Flask(__name__)

@app.route('/time')
def get_time():
    return utc_client.get_utc_time(UTC_SERVER, UTC_PORT)
```

### Monitoring Integration
```yaml
# Prometheus configuration
- job_name: 'simple-utcd'
  static_configs:
    - targets: ['localhost:8080']
  metrics_path: /metrics
  scrape_interval: 30s
```

## Next Steps

- **Configuration**: See [Configuration Guide](../configuration/README.md)
- **Deployment**: See [Deployment Guide](../deployment/README.md)
- **Troubleshooting**: See [Troubleshooting Guide](../troubleshooting/README.md)
- **CLI Reference**: See [CLI Reference](cli.md)

---

*This user guide provides comprehensive information for using Simple UTC Daemon effectively.*
