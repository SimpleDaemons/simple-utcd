# Simple UTC Daemon - Deployment Guide

This directory contains deployment configurations and scripts for the Simple UTC Daemon across different platforms and environments.

## Directory Structure

```
deployment/
├── configs/              # Configuration templates
├── examples/             # Deployment examples
│   ├── docker/          # Docker deployment
│   ├── kubernetes/      # Kubernetes deployment
│   └── monitoring/      # Monitoring setup
├── init.d/              # Traditional Linux init scripts
├── launchd/             # macOS launchd configuration
├── systemd/             # systemd service files
├── templates/           # Configuration templates
└── windows/             # Windows service configuration
```

## Platform Support

### Linux (systemd)
- **Service Files**: `systemd/simple-utcd.service`
- **Socket Activation**: `systemd/simple-utcd.socket`
- **Target**: `systemd/simple-utcd.target`
- **Installation**: `sudo cp systemd/*.service /etc/systemd/system/`
- **Management**: `systemctl start/stop/restart simple-utcd`

### Linux (Traditional)
- **Init Script**: `init.d/simple-utcd`
- **Installation**: `sudo cp init.d/simple-utcd /etc/init.d/`
- **Management**: `service simple-utcd start/stop/restart`

### macOS (launchd)
- **Plist File**: `launchd/com.simpledaemons.simple-utcd.plist`
- **Installation**: `sudo cp launchd/*.plist /Library/LaunchDaemons/`
- **Management**: `sudo launchctl load/unload`

### Windows
- **Batch Script**: `windows/install-service.bat`
- **PowerShell Script**: `windows/install-service.ps1`
- **Service Binary**: `windows/simple-utcd-service.exe` (to be built)

## Quick Start

### Linux (systemd)
```bash
# Install service files
sudo cp deployment/systemd/*.service /etc/systemd/system/
sudo cp deployment/systemd/*.socket /etc/systemd/system/
sudo cp deployment/systemd/*.target /etc/systemd/system/

# Reload systemd
sudo systemctl daemon-reload

# Enable and start service
sudo systemctl enable simple-utcd
sudo systemctl start simple-utcd

# Check status
sudo systemctl status simple-utcd
```

### macOS (launchd)
```bash
# Install plist file
sudo cp deployment/launchd/com.simpledaemons.simple-utcd.plist /Library/LaunchDaemons/

# Load and start service
sudo launchctl load /Library/LaunchDaemons/com.simpledaemons.simple-utcd.plist

# Check status
sudo launchctl list | grep simple-utcd
```

### Windows
```cmd
# Using batch script
deployment\windows\install-service.bat install
deployment\windows\install-service.bat config
deployment\windows\install-service.bat start

# Using PowerShell
deployment\windows\install-service.ps1 -Action install
deployment\windows\install-service.ps1 -Action config
deployment\windows\install-service.ps1 -Action start
```

## Configuration

### Default Configuration
- **Listen Address**: 0.0.0.0 (all interfaces)
- **Listen Port**: 37 (UTC protocol)
- **IPv6**: Enabled
- **Max Connections**: 1000
- **Log Level**: INFO
- **Worker Threads**: 4

### Configuration Files
- **Linux/macOS**: `/etc/simple-utcd/simple-utcd.conf`
- **Windows**: `%ProgramData%\SimpleUtcd\simple-utcd.conf`

## Security Considerations

### Service User
- **Linux**: `utc` user and group
- **macOS**: `_utc` user and group
- **Windows**: `NT AUTHORITY\SYSTEM`

### File Permissions
- Configuration files: 644 (readable by service user)
- Log files: 644 (writable by service user)
- Data files: 755 (writable by service user)

### Network Security
- Service binds to all interfaces by default
- Consider restricting to specific interfaces in production
- Firewall rules may be needed for port 37

## Monitoring

### Log Files
- **Linux**: `/var/log/simple-utcd/simple-utcd.log`
- **macOS**: `/usr/local/var/log/simple-utcd/simple-utcd.log`
- **Windows**: `%ProgramData%\SimpleUtcd\logs\simple-utcd.log`

### Health Checks
- Service status monitoring
- Port 37 availability
- Configuration validation
- Log file rotation

## Troubleshooting

### Common Issues

1. **Service won't start**
   - Check configuration file syntax
   - Verify user permissions
   - Check port availability

2. **Permission denied**
   - Ensure service user exists
   - Check file permissions
   - Verify directory ownership

3. **Port already in use**
   - Check if another service is using port 37
   - Use `netstat` or `ss` to check port usage
   - Consider changing port in configuration

### Debug Mode
```bash
# Run in foreground for debugging
simple-utcd -c /etc/simple-utcd/simple-utcd.conf -d

# Check configuration
simple-utcd -t -c /etc/simple-utcd/simple-utcd.conf
```

## Advanced Deployment

### Docker
See `examples/docker/` for containerized deployment options.

### Kubernetes
See `examples/kubernetes/` for Kubernetes deployment manifests.

### Monitoring
See `examples/monitoring/` for Prometheus and Grafana setup.

## Support

For deployment issues:
1. Check the logs
2. Verify configuration
3. Test connectivity
4. Review security settings
5. Open an issue on GitHub

---

*This deployment guide is part of the Simple UTC Daemon project.*
