# Deployment Guide

This guide covers deployment strategies and best practices for Simple UTC Daemon in various environments.

## Deployment Overview

Simple UTC Daemon can be deployed in multiple ways depending on your requirements:

- **Single Instance**: Basic deployment for small environments
- **High Availability**: Multiple instances with load balancing
- **Containerized**: Docker and Kubernetes deployments
- **Cloud**: AWS, Azure, GCP deployments

## Docker Deployment (Recommended)

For most use cases, Docker deployment is the recommended approach:

### Quick Start with Docker

```bash
# Navigate to Docker examples
cd deployment/examples/docker

# Create configuration directory
mkdir -p config logs
cp ../../config/examples/simple/simple-utcd.conf.example config/simple-utcd.conf

# Start the service
docker-compose up -d

# Check status
docker-compose ps
```

### Docker Features

- **Multi-stage builds** for optimized production images
- **Multi-platform support** (Ubuntu, CentOS, Alpine)
- **Multi-architecture builds** (x86_64, ARM64, ARMv7)
- **Development environment** with debugging tools
- **Health checks** and monitoring
- **Volume mounts** for configuration and logs
- **Automated build scripts** for cross-platform building

### Docker Commands

```bash
# Development environment
docker-compose --profile dev up dev

# Runtime environment
docker-compose --profile runtime up simple-utcd

# Build for all distributions
./scripts/build-docker.sh -d all

# Deploy with custom settings
./scripts/deploy-docker.sh -p runtime -c ./config -l ./logs
```

For comprehensive Docker documentation, see [Docker Deployment Guide](docker.md).

## Deployment Strategies

### Single Instance Deployment

#### Linux/macOS
```bash
# Install daemon
sudo make install

# Create configuration
sudo cp config/examples/simple/simple-utcd.conf.example /etc/simple-utcd/simple-utcd.conf

# Create system user
sudo useradd -r -s /bin/false -d /var/lib/simple-utcd simple-utcd

# Set permissions
sudo chown -R simple-utcd:simple-utcd /var/log/simple-utcd /var/lib/simple-utcd

# Enable and start service
sudo systemctl enable simple-utcd
sudo systemctl start simple-utcd
```

#### Windows
```cmd
# Install service
install-service.bat

# Configure
copy config\examples\simple\simple-utcd.conf.example C:\ProgramData\Simple-UTCd\simple-utcd.conf

# Start service
sc start Simple-UTCd
```

### High Availability Deployment

#### Load Balancer Configuration
```nginx
# Nginx configuration
upstream utc_servers {
    server 192.168.1.10:37;
    server 192.168.1.11:37;
    server 192.168.1.12:37;
}

server {
    listen 37;
    proxy_pass utc_servers;
}
```

#### Health Checks
```bash
#!/bin/bash
# Health check script
check_utc_server() {
    local host=$1
    local port=$2

    if nc -z $host $port; then
        echo "✅ $host:$port is healthy"
        return 0
    else
        echo "❌ $host:$port is unhealthy"
        return 1
    fi
}

# Check all servers
check_utc_server 192.168.1.10 37
check_utc_server 192.168.1.11 37
check_utc_server 192.168.1.12 37
```

### Containerized Deployment

#### Docker (Multi-stage Build)
The project includes a comprehensive multi-stage Dockerfile supporting multiple distributions and architectures:

```bash
# Build for specific distribution
docker-compose --profile build build build-ubuntu
docker-compose --profile build build build-centos
docker-compose --profile build build build-alpine

# Build for all distributions
./scripts/build-docker.sh -d all
```

#### Docker Compose (Production)
```yaml
version: '3.8'

services:
  simple-utcd:
    build:
      context: .
      dockerfile: Dockerfile
      target: runtime
    ports:
      - "37:37/tcp"
    volumes:
      - ./config:/etc/simple-utcd:ro
      - ./logs:/var/log/simple-utcd
    environment:
      - SIMPLE_UTCD_LOG_LEVEL=INFO
    restart: unless-stopped
    healthcheck:
      test: ["CMD", "nc", "-z", "localhost", "37"]
      interval: 30s
      timeout: 10s
      retries: 3
    networks:
      - utc-network

networks:
  utc-network:
    driver: bridge
```

#### Docker Compose (Development)
```yaml
version: '3.8'

services:
  dev:
    build:
      context: .
      dockerfile: Dockerfile
      target: dev
    volumes:
      - .:/app
      - build-cache:/app/build
    ports:
      - "37:37/tcp"
    environment:
      - PLATFORM=linux
      - DISTRO=ubuntu
    command: /bin/bash
    profiles:
      - dev
```

#### Kubernetes
```yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: simple-utcd
spec:
  replicas: 3
  selector:
    matchLabels:
      app: simple-utcd
  template:
    metadata:
      labels:
        app: simple-utcd
    spec:
      containers:
      - name: simple-utcd
        image: simple-utcd:latest
        ports:
        - containerPort: 37
          protocol: UDP
        - containerPort: 37
          protocol: TCP
        volumeMounts:
        - name: config
          mountPath: /etc/simple-utcd
        - name: logs
          mountPath: /var/log/simple-utcd
      volumes:
      - name: config
        configMap:
          name: simple-utcd-config
      - name: logs
        emptyDir: {}
---
apiVersion: v1
kind: Service
metadata:
  name: simple-utcd-service
spec:
  selector:
    app: simple-utcd
  ports:
  - port: 37
    targetPort: 37
    protocol: UDP
  - port: 37
    targetPort: 37
    protocol: TCP
  type: LoadBalancer
```

## Environment-Specific Deployments

### Development Environment
```ini
# Development configuration
listen_address = 127.0.0.1
listen_port = 1037
log_level = DEBUG
enable_console_logging = true
enable_authentication = false
worker_threads = 1
max_connections = 10
```

### Staging Environment
```ini
# Staging configuration
listen_address = 0.0.0.0
listen_port = 37
log_level = INFO
enable_console_logging = false
enable_syslog = true
enable_authentication = true
authentication_key = staging-key
worker_threads = 2
max_connections = 100
```

### Production Environment
```ini
# Production configuration
listen_address = 0.0.0.0
listen_port = 37
log_level = WARN
enable_console_logging = false
enable_syslog = true
enable_authentication = true
authentication_key = production-secure-key
allowed_clients = ["192.168.0.0/16", "10.0.0.0/8"]
worker_threads = 8
max_connections = 10000
enable_statistics = true
stats_interval = 60
```

## Monitoring and Observability

### Health Checks
```bash
#!/bin/bash
# Comprehensive health check
check_utc_health() {
    local host=${1:-localhost}
    local port=${2:-37}

    # Check if service is running
    if ! pgrep -f simple-utcd > /dev/null; then
        echo "❌ Simple UTC Daemon process not running"
        return 1
    fi

    # Check if port is listening
    if ! nc -z $host $port; then
        echo "❌ Port $port not listening"
        return 1
    fi

    # Check if service responds
    if ! timeout 5 bash -c "echo '' | nc $host $port" > /dev/null; then
        echo "❌ Service not responding"
        return 1
    fi

    echo "✅ Simple UTC Daemon is healthy"
    return 0
}
```

### Metrics Collection
```bash
#!/bin/bash
# Collect metrics
collect_metrics() {
    local log_file="/var/log/simple-utcd/simple-utcd.log"

    # Count connections
    local connections=$(grep "New connection" $log_file | wc -l)

    # Count errors
    local errors=$(grep "ERROR" $log_file | wc -l)

    # Get uptime
    local uptime=$(ps -o etime= -p $(pgrep simple-utcd) | tr -d ' ')

    echo "connections_total $connections"
    echo "errors_total $errors"
    echo "uptime_seconds $uptime"
}
```

### Log Monitoring
```bash
#!/bin/bash
# Monitor logs for issues
monitor_logs() {
    local log_file="/var/log/simple-utcd/simple-utcd.log"

    # Monitor for errors
    tail -f $log_file | grep --line-buffered "ERROR" | while read line; do
        echo "🚨 ERROR detected: $line"
        # Send alert (email, Slack, etc.)
    done

    # Monitor for high connection counts
    tail -f $log_file | grep --line-buffered "connection" | while read line; do
        if [[ $line =~ "active: ([0-9]+)" ]]; then
            local count=${BASH_REMATCH[1]}
            if [ $count -gt 1000 ]; then
                echo "⚠️  High connection count: $count"
            fi
        fi
    done
}
```

## Security Considerations

### Network Security
```bash
# Firewall configuration
sudo ufw allow from 192.168.0.0/16 to any port 37
sudo ufw allow from 10.0.0.0/8 to any port 37
sudo ufw deny 37

# Or with iptables
sudo iptables -A INPUT -p udp --dport 37 -s 192.168.0.0/16 -j ACCEPT
sudo iptables -A INPUT -p tcp --dport 37 -s 192.168.0.0/16 -j ACCEPT
sudo iptables -A INPUT -p udp --dport 37 -j DROP
sudo iptables -A INPUT -p tcp --dport 37 -j DROP
```

### Authentication
```ini
# Enable authentication
enable_authentication = true
authentication_key = your-secure-key-here

# Restrict access
allowed_clients = ["192.168.0.0/16", "10.0.0.0/8"]
denied_clients = ["192.168.1.100"]
```

### SSL/TLS (Future Feature)
```ini
# SSL/TLS configuration (when implemented)
enable_ssl = true
ssl_cert = /etc/ssl/certs/simple-utcd.crt
ssl_key = /etc/ssl/private/simple-utcd.key
ssl_ca = /etc/ssl/certs/ca.crt
```

## Backup and Recovery

### Configuration Backup
```bash
#!/bin/bash
# Backup configuration
backup_config() {
    local backup_dir="/var/backups/simple-utcd"
    local timestamp=$(date +%Y%m%d_%H%M%S)

    mkdir -p $backup_dir

    # Backup configuration
    cp /etc/simple-utcd/simple-utcd.conf $backup_dir/simple-utcd.conf.$timestamp

    # Backup logs
    cp /var/log/simple-utcd/simple-utcd.log $backup_dir/simple-utcd.log.$timestamp

    # Keep only last 30 days
    find $backup_dir -name "*.conf.*" -mtime +30 -delete
    find $backup_dir -name "*.log.*" -mtime +30 -delete
}
```

### Disaster Recovery
```bash
#!/bin/bash
# Disaster recovery procedure
disaster_recovery() {
    # Stop service
    sudo systemctl stop simple-utcd

    # Restore configuration
    cp /var/backups/simple-utcd/simple-utcd.conf.latest /etc/simple-utcd/simple-utcd.conf

    # Restore logs
    cp /var/backups/simple-utcd/simple-utcd.log.latest /var/log/simple-utcd/simple-utcd.log

    # Start service
    sudo systemctl start simple-utcd

    # Verify
    sleep 5
    if systemctl is-active --quiet simple-utcd; then
        echo "✅ Recovery successful"
    else
        echo "❌ Recovery failed"
        exit 1
    fi
}
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
echo "net.ipv4.udp_mem = 1024 87380 134217728" >> /etc/sysctl.conf
sysctl -p
```

### Application Tuning
```ini
# High-performance configuration
worker_threads = 16
max_connections = 10000
timeout = 500
sync_interval = 32
enable_statistics = true
stats_interval = 30
```

## Troubleshooting Deployment

### Common Issues

#### Service Won't Start
```bash
# Check configuration
simple-utcd --config-test -c /etc/simple-utcd/simple-utcd.conf

# Check logs
journalctl -u simple-utcd -f

# Check permissions
ls -la /etc/simple-utcd/simple-utcd.conf
```

#### High Memory Usage
```bash
# Check memory usage
ps aux | grep simple-utcd

# Check for memory leaks
valgrind --tool=memcheck simple-utcd -c /etc/simple-utcd/simple-utcd.conf
```

#### Network Issues
```bash
# Check if port is listening
sudo netstat -tulpn | grep :37

# Check firewall
sudo ufw status
sudo iptables -L

# Test connectivity
nc -zv localhost 37
```

## Next Steps

- **Docker Deployment**: See [Docker Deployment Guide](docker.md) for comprehensive Docker setup
- **Production Setup**: See [Production Setup Guide](production.md)
- **Examples**: See [Deployment Examples](../examples/deployment.md)
- **Troubleshooting**: See [Troubleshooting Guide](../troubleshooting/README.md)

---

*Next: [Docker Deployment Guide](docker.md) for containerized deployment or [Production Setup Guide](production.md) for enterprise-grade deployment*
