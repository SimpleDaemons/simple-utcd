# Docker Deployment Guide

This guide covers deploying simple-utcd using Docker containers.

## Quick Start

### 1. Clone the Repository

```bash
git clone <repository-url>
cd simple-utcd
```

### 2. Create Configuration Directory

```bash
mkdir -p deployment/examples/docker/config
mkdir -p deployment/examples/docker/logs
cp config/examples/simple/simple-utcd.conf.example deployment/examples/docker/config/simple-utcd.conf
```

### 3. Build and Run

```bash
cd deployment/examples/docker
docker-compose up -d
```

### 4. Verify Deployment

```bash
# Check container status
docker-compose ps

# Check logs
docker-compose logs -f simple-utcd

# Test UTC service
nc -z localhost 37
```

## Configuration

### Environment Variables

The following environment variables can be set in the docker-compose.yml:

- `SIMPLE_UTCD_LISTEN_ADDRESS`: Bind address (default: 0.0.0.0)
- `SIMPLE_UTCD_LISTEN_PORT`: Bind port (default: 37)
- `SIMPLE_UTCD_LOG_LEVEL`: Log level (default: INFO)

### Volume Mounts

- `./config:/etc/simple-utcd:ro`: Configuration files (read-only)
- `./logs:/var/log/simple-utcd`: Log files

## Production Deployment

### 1. Create Production Configuration

```bash
# Copy production config
cp config/examples/production/enterprise.conf deployment/examples/docker/config/simple-utcd.conf

# Edit configuration
nano deployment/examples/docker/config/simple-utcd.conf
```

### 2. Set Resource Limits

```yaml
services:
  simple-utcd:
    # ... existing configuration ...
    deploy:
      resources:
        limits:
          memory: 256M
          cpus: '0.5'
        reservations:
          memory: 128M
          cpus: '0.25'
```

### 3. Enable Logging Driver

```yaml
services:
  simple-utcd:
    # ... existing configuration ...
    logging:
      driver: "json-file"
      options:
        max-size: "10m"
        max-file: "3"
```

## Monitoring

### Health Checks

The container includes a health check that verifies UTC service availability:

```bash
# Check health status
docker inspect --format='{{.State.Health.Status}}' simple-utcd
```

### Log Monitoring

```bash
# Follow logs in real-time
docker-compose logs -f simple-utcd

# Search logs for errors
docker-compose logs simple-utcd | grep ERROR

# Export logs
docker-compose logs simple-utcd > utc-logs.txt
```

## Troubleshooting

### Common Issues

1. **Port Already in Use**
   ```bash
   # Check what's using port 37
   sudo lsof -i :37

   # Stop conflicting service
   sudo systemctl stop time
   ```

2. **Permission Issues**
   ```bash
   # Fix log directory permissions
   sudo chown -R 1000:1000 deployment/examples/docker/logs
   ```

3. **Configuration Errors**
   ```bash
   # Validate configuration
   docker-compose exec simple-utcd simple-utcd --test-config
   ```

### Debug Mode

```bash
# Run with debug logging
docker-compose run --rm simple-utcd simple-utcd --verbose --foreground
```

## Scaling

### Multiple Instances

```yaml
services:
  simple-utcd:
    # ... existing configuration ...
    deploy:
      replicas: 3
      endpoint_mode: dnsrr
```

### Load Balancing

Use an external load balancer (HAProxy, nginx) to distribute UTC requests across multiple containers.

## Security Considerations

- The container runs as a non-root user
- Configuration files are mounted as read-only
- Only TCP port 37 is exposed
- Network isolation using custom bridge network

## Backup and Recovery

### Backup Configuration

```bash
# Backup configuration
docker cp simple-utcd:/etc/simple-utcd/simple-utcd.conf ./backup/
```

### Restore Configuration

```bash
# Restore configuration
docker cp ./backup/simple-utcd.conf simple-utcd:/etc/simple-utcd/
docker-compose restart simple-utcd
```

## Performance Tuning

### Container Optimization

```yaml
services:
  simple-utcd:
    # ... existing configuration ...
    ulimits:
      nofile:
        soft: 65536
        hard: 65536
    sysctls:
      - net.core.rmem_max=26214400
      - net.core.wmem_max=26214400
```

### Host System Tuning

```bash
# Increase TCP buffer sizes
echo 'net.core.rmem_max=26214400' >> /etc/sysctl.conf
echo 'net.core.wmem_max=26214400' >> /etc/sysctl.conf
sysctl -p
```
