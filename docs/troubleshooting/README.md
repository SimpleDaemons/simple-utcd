# Troubleshooting Guide

This guide helps you diagnose and resolve common issues with Simple UTC Daemon.

## Quick Diagnostics

### Service Status Check
```bash
# Check if service is running
systemctl status simple-utcd

# Check process
ps aux | grep simple-utcd

# Check port listening
netstat -tulpn | grep :37
```

### Configuration Validation
```bash
# Test configuration syntax
simple-utcd --config-test -c /etc/simple-utcd/simple-utcd.conf

# Check configuration file permissions
ls -la /etc/simple-utcd/simple-utcd.conf
```

### Basic Connectivity Test
```bash
# Test local connectivity
nc -zv localhost 37

# Test remote connectivity
nc -zv 192.168.1.100 37

# Test with telnet
telnet localhost 37
```

## Common Issues and Solutions

### Service Won't Start

#### Issue: Permission Denied
**Symptoms:**
```
[ERROR] Failed to bind to port 37: Permission denied
```

**Solutions:**
```bash
# Run as root (required for port 37)
sudo simple-utcd -c /etc/simple-utcd/simple-utcd.conf

# Or change to non-privileged port
listen_port = 1037
```

#### Issue: Port Already in Use
**Symptoms:**
```
[ERROR] Failed to bind to port 37: Address already in use
```

**Solutions:**
```bash
# Check what's using the port
sudo netstat -tulpn | grep :37
sudo lsof -i :37

# Kill conflicting process
sudo kill -9 <PID>

# Or change port
listen_port = 1037
```

#### Issue: Configuration File Not Found
**Symptoms:**
```
[ERROR] Failed to load configuration file
```

**Solutions:**
```bash
# Check if file exists
ls -la /etc/simple-utcd/simple-utcd.conf

# Use absolute path
simple-utcd -c /full/path/to/simple-utcd.conf

# Copy example configuration
sudo cp config/examples/simple/simple-utcd.conf.example /etc/simple-utcd/simple-utcd.conf
```

### Service Starts but Doesn't Respond

#### Issue: Firewall Blocking
**Symptoms:**
- Service starts successfully
- No response to connections
- Connection timeouts

**Solutions:**
```bash
# Check firewall status
sudo ufw status
sudo firewall-cmd --list-ports

# Allow port 37
sudo ufw allow 37
sudo firewall-cmd --permanent --add-port=37/udp
sudo firewall-cmd --permanent --add-port=37/tcp
sudo firewall-cmd --reload
```

#### Issue: Wrong Listen Address
**Symptoms:**
- Service binds to localhost only
- Remote connections fail

**Solutions:**
```ini
# Change listen address
listen_address = 0.0.0.0  # Listen on all interfaces
listen_address = 192.168.1.100  # Listen on specific interface
```

#### Issue: IPv6 Issues
**Symptoms:**
- IPv4 connections work
- IPv6 connections fail

**Solutions:**
```ini
# Disable IPv6 if not needed
enable_ipv6 = false

# Or check IPv6 configuration
ip -6 addr show
```

### Performance Issues

#### Issue: High CPU Usage
**Symptoms:**
- High CPU utilization
- Slow response times

**Solutions:**
```bash
# Check CPU usage
top -p $(pgrep simple-utcd)

# Reduce worker threads
worker_threads = 2

# Check for infinite loops in logs
grep -i "loop\|infinite" /var/log/simple-utcd/simple-utcd.log
```

#### Issue: High Memory Usage
**Symptoms:**
- High memory consumption
- System slowdown

**Solutions:**
```bash
# Check memory usage
ps aux | grep simple-utcd

# Check for memory leaks
valgrind --tool=memcheck simple-utcd -c /etc/simple-utcd/simple-utcd.conf

# Reduce connection limits
max_connections = 100
```

#### Issue: Connection Limits
**Symptoms:**
- New connections rejected
- "Too many connections" errors

**Solutions:**
```bash
# Check current connections
netstat -an | grep :37 | wc -l

# Increase connection limit
max_connections = 1000

# Check system limits
ulimit -n
```

### Authentication Issues

#### Issue: Authentication Failures
**Symptoms:**
- Clients can't authenticate
- "Authentication failed" errors

**Solutions:**
```bash
# Check authentication key
grep authentication_key /etc/simple-utcd/simple-utcd.conf

# Verify key matches client configuration
# Check for typos in key

# Disable authentication for testing
enable_authentication = false
```

#### Issue: Access Control Issues
**Symptoms:**
- Legitimate clients denied
- "Access denied" errors

**Solutions:**
```bash
# Check allowed clients list
grep allowed_clients /etc/simple-utcd/simple-utcd.conf

# Add client IP to allowed list
allowed_clients = ["192.168.1.100", "10.0.0.50"]

# Check denied clients list
grep denied_clients /etc/simple-utcd/simple-utcd.conf
```

### Logging Issues

#### Issue: No Logs Generated
**Symptoms:**
- Service runs but no log files
- Empty log files

**Solutions:**
```bash
# Check log file path
grep log_file /etc/simple-utcd/simple-utcd.conf

# Check directory permissions
ls -la /var/log/simple-utcd/

# Create log directory
sudo mkdir -p /var/log/simple-utcd
sudo chown simple-utcd:simple-utcd /var/log/simple-utcd
```

#### Issue: Log Rotation Problems
**Symptoms:**
- Log files grow too large
- Old logs not rotated

**Solutions:**
```bash
# Check logrotate configuration
cat /etc/logrotate.d/simple-utcd

# Manual log rotation
sudo logrotate -f /etc/logrotate.d/simple-utcd

# Check disk space
df -h /var/log/simple-utcd/
```

## Debugging Techniques

### Enable Debug Logging
```ini
# Enable verbose logging
log_level = DEBUG
enable_console_logging = true

# Run in debug mode
simple-utcd -c /etc/simple-utcd/simple-utcd.conf --debug
```

### Network Debugging
```bash
# Monitor network traffic
sudo tcpdump -i any port 37

# Check routing
ip route show

# Test DNS resolution
nslookup 192.168.1.100
```

### System Resource Monitoring
```bash
# Monitor system resources
htop
iotop
nethogs

# Check system logs
journalctl -u simple-utcd -f
dmesg | tail -20
```

### Application Debugging
```bash
# Run with strace
strace -f simple-utcd -c /etc/simple-utcd/simple-utcd.conf

# Run with gdb
gdb simple-utcd
(gdb) run -c /etc/simple-utcd/simple-utcd.conf
```

## Error Codes and Meanings

### Common Error Codes
- **EACCES (13)**: Permission denied
- **EADDRINUSE (98)**: Address already in use
- **ENOENT (2)**: No such file or directory
- **ENOMEM (12)**: Out of memory
- **ECONNREFUSED (111)**: Connection refused
- **ETIMEDOUT (110)**: Connection timed out

### Error Code Lookup
```bash
# Look up error code
errno 13
errno 98

# Or use perror
python3 -c "import os; print(os.strerror(13))"
```

## Performance Troubleshooting

### Slow Response Times
```bash
# Check response times
time nc -z localhost 37

# Monitor system load
uptime
top

# Check network latency
ping -c 10 localhost
```

### High Connection Count
```bash
# Count active connections
netstat -an | grep :37 | grep ESTABLISHED | wc -l

# Check connection states
netstat -an | grep :37 | awk '{print $6}' | sort | uniq -c
```

### Memory Leaks
```bash
# Monitor memory usage over time
while true; do
    ps -o rss= -p $(pgrep simple-utcd)
    sleep 60
done

# Use valgrind for detailed analysis
valgrind --tool=memcheck --leak-check=full simple-utcd -c /etc/simple-utcd/simple-utcd.conf
```

## Recovery Procedures

### Service Recovery
```bash
# Restart service
sudo systemctl restart simple-utcd

# Reload configuration
sudo kill -HUP $(pgrep simple-utcd)

# Full restart with cleanup
sudo systemctl stop simple-utcd
sudo pkill -f simple-utcd
sudo systemctl start simple-utcd
```

### Configuration Recovery
```bash
# Restore from backup
sudo cp /var/backups/simple-utcd/simple-utcd.conf.backup /etc/simple-utcd/simple-utcd.conf

# Use default configuration
sudo cp config/examples/simple/simple-utcd.conf.example /etc/simple-utcd/simple-utcd.conf
```

### Data Recovery
```bash
# Restore logs
sudo cp /var/backups/simple-utcd/logs.tar.gz /tmp/
cd /tmp && sudo tar -xzf logs.tar.gz

# Restore statistics
sudo cp -r /var/backups/simple-utcd/stats/ /var/lib/simple-utcd/
```

## Getting Help

### Information to Collect
When reporting issues, collect:

1. **System Information**:
   ```bash
   uname -a
   cat /etc/os-release
   ```

2. **Service Status**:
   ```bash
   systemctl status simple-utcd
   journalctl -u simple-utcd --no-pager
   ```

3. **Configuration**:
   ```bash
   cat /etc/simple-utcd/simple-utcd.conf
   ```

4. **Logs**:
   ```bash
   tail -100 /var/log/simple-utcd/simple-utcd.log
   ```

5. **Network Information**:
   ```bash
   netstat -tulpn | grep :37
   ss -tulpn | grep :37
   ```

### Support Channels
- **GitHub Issues**: [Report bugs and request features](https://github.com/simpledaemons/simple-utcd/issues)
- **GitHub Discussions**: [Community support](https://github.com/simpledaemons/simple-utcd/discussions)
- **Documentation**: [Full documentation](../README.md)

### Escalation Process
1. Check this troubleshooting guide
2. Search existing GitHub issues
3. Create a new issue with collected information
4. Provide detailed reproduction steps
5. Include system and configuration details

## Prevention

### Best Practices
- Regular configuration backups
- Monitor system resources
- Keep logs rotated
- Test configuration changes
- Use version control for configurations
- Monitor service health
- Regular security updates

### Monitoring Setup
```bash
# Set up basic monitoring
crontab -e

# Add health check every 5 minutes
*/5 * * * * /usr/local/bin/check-utc-health.sh
```

---

*For more help, see the [Support Guide](../support/README.md) or [GitHub Issues](https://github.com/simpledaemons/simple-utcd/issues)*
