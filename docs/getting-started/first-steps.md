# First Steps Guide

This guide will walk you through your first configuration and test of Simple UTC Daemon after installation.

## Prerequisites

- Simple UTC Daemon installed (see [Installation Guide](installation.md))
- Basic understanding of UTC protocol
- Access to system configuration files

## Step 1: Initial Configuration

### 1.1 Locate Configuration File
```bash
# Default locations
# Linux/macOS: /etc/simple-utcd/simple-utcd.conf
# Windows: C:\ProgramData\Simple-UTCd\simple-utcd.conf
```

### 1.2 Basic Configuration
Edit the configuration file with your preferred editor:

```bash
# Linux/macOS
sudo nano /etc/simple-utcd/simple-utcd.conf

# Windows
notepad C:\ProgramData\Simple-UTCd\simple-utcd.conf
```

### 1.3 Essential Settings
```ini
# Network Configuration
listen_address = 0.0.0.0    # Listen on all interfaces
listen_port = 37            # Standard UTC port
enable_ipv6 = true          # Enable IPv6 support

# Basic Security
enable_authentication = false  # Disable for testing
max_connections = 100          # Limit concurrent connections

# Logging
log_level = INFO
enable_console_logging = true
log_file = /var/log/simple-utcd/simple-utcd.log

# Performance
worker_threads = 2
enable_statistics = true
```

## Step 2: Start the Service

### 2.1 Test Configuration
```bash
# Test configuration syntax
simple-utcd --config-test -c /etc/simple-utcd/simple-utcd.conf
```

### 2.2 Start in Foreground (Testing)
```bash
# Linux/macOS
sudo simple-utcd -c /etc/simple-utcd/simple-utcd.conf

# Windows (as Administrator)
simple-utcd.exe -c C:\ProgramData\Simple-UTCd\simple-utcd.conf
```

Expected output:
```
[2025-01-01 12:00:00.000] [INFO] Simple UTC Daemon starting...
[2025-01-01 12:00:00.001] [INFO] UTC Server initialized
[2025-01-01 12:00:00.002] [INFO] UTC Daemon initialized successfully
[2025-01-01 12:00:00.003] [INFO] Listening on 0.0.0.0:37
[2025-01-01 12:00:00.004] [INFO] Starting UTC Server on 0.0.0.0:37
[2025-01-01 12:00:00.005] [INFO] UTC Server started successfully with 2 worker threads
[2025-01-01 12:00:00.006] [INFO] UTC Daemon is running. Press Ctrl+C to stop.
```

### 2.3 Start as Service
```bash
# Linux - systemd
sudo systemctl start simple-utcd
sudo systemctl enable simple-utcd

# macOS - launchd
sudo launchctl load /Library/LaunchDaemons/com.simpledaemons.simple-utcd.plist

# Windows
sc start Simple-UTCd
```

## Step 3: Test the Service

### 3.1 Basic Connectivity Test
```bash
# Test port availability
nc -zv localhost 37

# Expected output:
# Connection to localhost 37 port [tcp/time] succeeded!
```

### 3.2 UTC Time Request Test
```bash
# Using telnet
telnet localhost 37

# You should see 4 bytes of binary data
# Press Ctrl+] then type 'quit' to exit
```

### 3.3 Python Test Script
Create a test script to verify UTC functionality:

```python
#!/usr/bin/env python3
import socket
import struct
import time
from datetime import datetime

def test_utc_service(host='localhost', port=37):
    try:
        # Connect to UTC service
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(5)
        sock.connect((host, port))

        # Receive UTC timestamp (4 bytes)
        data = sock.recv(4)
        sock.close()

        if len(data) != 4:
            print(f"ERROR: Expected 4 bytes, got {len(data)}")
            return False

        # Convert from UTC epoch (1900) to Unix epoch (1970)
        utc_timestamp = struct.unpack('!I', data)[0]
        unix_timestamp = utc_timestamp - 2208988800

        # Convert to readable format
        utc_time = datetime.fromtimestamp(unix_timestamp, tz=time.timezone.utc)
        current_time = datetime.now(time.timezone.utc)

        # Check if time is reasonable (within 1 hour)
        time_diff = abs((utc_time - current_time).total_seconds())

        print(f"UTC Service Test Results:")
        print(f"  Received UTC timestamp: {utc_timestamp}")
        print(f"  Converted to Unix: {unix_timestamp}")
        print(f"  UTC Time: {utc_time}")
        print(f"  Current Time: {current_time}")
        print(f"  Time Difference: {time_diff:.2f} seconds")

        if time_diff < 3600:  # Within 1 hour
            print("  ✅ SUCCESS: UTC service is working correctly")
            return True
        else:
            print("  ⚠️  WARNING: Time difference is large")
            return False

    except Exception as e:
        print(f"  ❌ ERROR: {e}")
        return False

if __name__ == "__main__":
    test_utc_service()
```

Save as `test_utc.py` and run:
```bash
python3 test_utc.py
```

### 3.4 Command Line Test
```bash
# Using netcat to get raw data
echo "" | nc localhost 37 | hexdump -C

# Expected output (example):
# 00000000  65 8f 2a 00                                       |e.*.|
# 00000004
```

## Step 4: Monitor the Service

### 4.1 Check Service Status
```bash
# Linux - systemd
sudo systemctl status simple-utcd

# macOS - launchd
sudo launchctl list | grep simple-utcd

# Windows
sc query Simple-UTCd
```

### 4.2 Monitor Logs
```bash
# Linux/macOS
tail -f /var/log/simple-utcd/simple-utcd.log

# Windows
Get-Content C:\ProgramData\Simple-UTCd\logs\simple-utcd.log -Wait
```

### 4.3 Check Statistics
```bash
# If statistics are enabled
curl http://localhost:8080/stats  # If HTTP interface is enabled
```

## Step 5: Basic Troubleshooting

### 5.1 Common Issues

#### Service Won't Start
```bash
# Check configuration syntax
simple-utcd --config-test -c /etc/simple-utcd/simple-utcd.conf

# Check logs
journalctl -u simple-utcd -f

# Run in debug mode
simple-utcd -c /etc/simple-utcd/simple-utcd.conf --debug
```

#### Permission Denied
```bash
# Check if running as root (required for port 37)
sudo simple-utcd -c /etc/simple-utcd/simple-utcd.conf

# Or change to a higher port
listen_port = 1037
```

#### Port Already in Use
```bash
# Check what's using port 37
sudo netstat -tulpn | grep :37

# Kill conflicting process or change port
```

#### No Response from Service
```bash
# Check firewall
sudo ufw status
sudo firewall-cmd --list-ports

# Check if service is listening
sudo netstat -tulpn | grep simple-utcd
```

### 5.2 Debug Mode
```bash
# Run with verbose logging
simple-utcd -c /etc/simple-utcd/simple-utcd.conf --debug --verbose

# Check all log levels
log_level = DEBUG
```

## Step 6: Next Configuration Steps

### 6.1 Security Configuration
```ini
# Enable authentication
enable_authentication = true
authentication_key = your-secret-key

# Restrict access
allowed_clients = ["192.168.1.0/24", "10.0.0.0/8"]
denied_clients = ["192.168.1.100"]
```

### 6.2 Performance Tuning
```ini
# Increase worker threads
worker_threads = 4

# Increase connection limit
max_connections = 1000

# Enable statistics
enable_statistics = true
stats_interval = 60
```

### 6.3 Logging Configuration
```ini
# Structured logging
log_format = json
log_file = /var/log/simple-utcd/simple-utcd.log

# Log rotation
log_rotation = true
log_max_size = 100MB
log_max_files = 5
```

## Verification Checklist

- [ ] Service starts without errors
- [ ] Service listens on configured port
- [ ] UTC time requests return valid timestamps
- [ ] Logs show successful connections
- [ ] Service responds to multiple clients
- [ ] Statistics are being collected (if enabled)
- [ ] Service handles errors gracefully

## Next Steps

- **Production Setup**: See [Production Setup Guide](../deployment/production.md)
- **Advanced Configuration**: See [Configuration Guide](../configuration/README.md)
- **Monitoring**: See [Deployment Guide](../deployment/README.md)
- **Troubleshooting**: See [Troubleshooting Guide](../troubleshooting/README.md)

## Getting Help

If you encounter issues:

1. **Check the logs** for error messages
2. **Verify configuration** syntax
3. **Test connectivity** with basic tools
4. **Review documentation** for your specific scenario
5. **Search issues** on GitHub
6. **Create an issue** if the problem persists

---

*Next: [Configuration Guide](../configuration/README.md) for detailed configuration options*
