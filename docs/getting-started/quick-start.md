# Quick Start Guide

Get Simple UTC Daemon up and running in minutes with this quick start guide.

## Prerequisites

- **Operating System**: Windows 10+, macOS 10.15+, or Linux (Ubuntu 18.04+, CentOS 7+)
- **Memory**: 512 MB RAM minimum
- **Disk Space**: 100 MB free space
- **Network**: Port 37 (UTC) available

## Installation

### Option 1: Pre-built Binaries (Recommended)

Download the latest release for your platform:

```bash
# Linux/macOS
wget https://github.com/simpledaemons/simple-utcd/releases/latest/download/simple-utcd-linux-amd64.tar.gz
tar -xzf simple-utcd-linux-amd64.tar.gz
sudo cp simple-utcd /usr/local/bin/

# Windows
# Download simple-utcd-windows-amd64.zip and extract
```

### Option 2: Build from Source

```bash
git clone https://github.com/simpledaemons/simple-utcd.git
cd simple-utcd
make build
```

## Basic Configuration

1. **Create configuration directory**:
```bash
sudo mkdir -p /etc/simple-utcd
```

2. **Copy example configuration**:
```bash
sudo cp config/examples/simple/simple-utcd.conf.example /etc/simple-utcd/simple-utcd.conf
```

3. **Edit configuration** (optional):
```bash
sudo nano /etc/simple-utcd/simple-utcd.conf
```

## Starting the Service

### Linux/macOS
```bash
# Run in foreground (for testing)
sudo ./simple-utcd -c /etc/simple-utcd/simple-utcd.conf

# Run as daemon
sudo ./simple-utcd -c /etc/simple-utcd/simple-utcd.conf -d
```

### Windows
```cmd
# Run in foreground
simple-utcd.exe -c C:\ProgramData\Simple-UTCd\simple-utcd.conf

# Install as service
install-service.bat
```

## Testing the Service

### Test with telnet
```bash
# Connect to UTC service
telnet localhost 37

# You should see 4 bytes of binary data representing UTC time
```

### Test with Python
```python
import socket
import struct
import time

# Connect to UTC service
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect(('localhost', 37))

# Receive UTC timestamp
data = sock.recv(4)
sock.close()

# Convert to Unix timestamp
utc_timestamp = struct.unpack('!I', data)[0]
unix_timestamp = utc_timestamp - 2208988800  # Convert from 1900 to 1970 epoch

print(f"UTC Time: {time.ctime(unix_timestamp)}")
```

### Test with curl (if HTTP interface is enabled)
```bash
curl http://localhost:8080/health
```

## Verification

1. **Check if service is running**:
```bash
# Linux/macOS
ps aux | grep simple-utcd

# Windows
tasklist | findstr simple-utcd
```

2. **Check logs**:
```bash
# Linux/macOS
tail -f /var/log/simple-utcd/simple-utcd.log

# Windows
type C:\ProgramData\Simple-UTCd\logs\simple-utcd.log
```

3. **Test connectivity**:
```bash
# Test port 37
nc -zv localhost 37
```

## Next Steps

- **Configure for production**: See [Production Setup](../deployment/production.md)
- **Advanced configuration**: See [Configuration Guide](../configuration/README.md)
- **Troubleshooting**: See [Troubleshooting Guide](../troubleshooting/README.md)

## Common Issues

### Port 37 Permission Denied
```bash
# Run with sudo (Linux/macOS)
sudo ./simple-utcd -c /etc/simple-utcd/simple-utcd.conf

# Or change port in configuration
listen_port = 1037
```

### Configuration File Not Found
```bash
# Check file exists
ls -la /etc/simple-utcd/simple-utcd.conf

# Use absolute path
./simple-utcd -c /full/path/to/simple-utcd.conf
```

### Service Won't Start
```bash
# Check logs for errors
tail -f /var/log/simple-utcd/simple-utcd.log

# Run in foreground for debugging
./simple-utcd -c /etc/simple-utcd/simple-utcd.conf --debug
```

## Getting Help

- **Documentation**: [Full Documentation](../README.md)
- **Issues**: [GitHub Issues](https://github.com/simpledaemons/simple-utcd/issues)
- **Community**: [Discussions](https://github.com/simpledaemons/simple-utcd/discussions)

---

*Next: [Installation Guide](installation.md) for detailed installation instructions*
