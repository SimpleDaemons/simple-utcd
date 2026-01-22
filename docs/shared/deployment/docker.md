# Docker Deployment Guide

This guide covers deploying simple-utcd using Docker containers for development, testing, and production environments.

## Overview

The Docker integration for simple-utcd provides:

- **Multi-stage builds** for different Linux distributions (Ubuntu, CentOS, Alpine)
- **Multi-architecture support** (x86_64, ARM64, ARMv7)
- **Development environment** with debugging tools
- **Production-ready runtime** with minimal footprint
- **Health checks** and monitoring capabilities
- **Volume mounts** for configuration and logs

## Quick Start

### Prerequisites

- Docker Engine 20.10+
- Docker Compose 2.0+
- Git

### 1. Clone and Setup

```bash
git clone <repository-url>
cd simple-utcd

# Create configuration directory
mkdir -p deployment/examples/docker/config
mkdir -p deployment/examples/docker/logs

# Copy example configuration
cp config/examples/simple/simple-utcd.conf.example deployment/examples/docker/config/simple-utcd.conf
```

### 2. Deploy with Docker Compose

```bash
# Navigate to Docker examples
cd deployment/examples/docker

# Start the service
docker-compose up -d

# Check status
docker-compose ps

# View logs
docker-compose logs -f simple-utcd
```

### 3. Test the Service

```bash
# Test UTC service connectivity
nc -z localhost 37

# Check health status
docker inspect --format='{{.State.Health.Status}}' simple-utcd
```

## Build Options

### Using Docker Compose Profiles

The project supports multiple build profiles:

```bash
# Development environment
docker-compose --profile dev up -d

# Runtime environment
docker-compose --profile runtime up -d

# Build for specific distributions
docker-compose --profile build build build-ubuntu
docker-compose --profile build build build-centos
docker-compose --profile build build build-alpine
```

### Using Build Scripts

```bash
# Build for all distributions
./scripts/build-docker.sh -d all

# Build for specific distribution
./scripts/build-docker.sh -d ubuntu -a x86_64

# Clean build
./scripts/build-docker.sh --clean
```

### Using Deployment Script

```bash
# Deploy runtime environment
./scripts/deploy-docker.sh -p runtime

# Deploy development environment
./scripts/deploy-docker.sh -p dev

# Force rebuild and deploy
./scripts/deploy-docker.sh --clean --force
```

## Configuration

### Environment Variables

Configure the service using environment variables:

```yaml
services:
  simple-utcd:
    environment:
      - SIMPLE_UTCD_LISTEN_ADDRESS=0.0.0.0
      - SIMPLE_UTCD_LISTEN_PORT=37
      - SIMPLE_UTCD_LOG_LEVEL=INFO
      - SIMPLE_UTCD_STRATUM=2
```

### Volume Mounts

Mount configuration and log directories:

```yaml
services:
  simple-utcd:
    volumes:
      - ./config:/etc/simple-utcd:ro
      - ./logs:/var/log/simple-utcd
```

### Network Configuration

Custom network for service isolation:

```yaml
networks:
  utc-network:
    driver: bridge
    ipam:
      config:
        - subnet: 172.21.0.0/16
```

## Production Deployment

### Resource Limits

Set appropriate resource limits:

```yaml
services:
  simple-utcd:
    deploy:
      resources:
        limits:
          memory: 256M
          cpus: '0.5'
        reservations:
          memory: 128M
          cpus: '0.25'
```

### Logging Configuration

Configure log rotation and retention:

```yaml
services:
  simple-utcd:
    logging:
      driver: "json-file"
      options:
        max-size: "10m"
        max-file: "3"
```

### Security Hardening

```yaml
services:
  simple-utcd:
    # Run as non-root user
    user: "1000:1000"

    # Read-only root filesystem
    read_only: true

    # Drop capabilities
    cap_drop:
      - ALL
    cap_add:
      - NET_BIND_SERVICE

    # Security options
    security_opt:
      - no-new-privileges:true
```

## Monitoring and Health Checks

### Built-in Health Checks

The container includes automatic health monitoring:

```yaml
healthcheck:
  test: ["CMD", "nc", "-z", "localhost", "37"]
  interval: 30s
  timeout: 10s
  retries: 3
  start_period: 40s
```

### Log Monitoring

```bash
# Follow logs in real-time
docker-compose logs -f simple-utcd

# Search for errors
docker-compose logs simple-utcd | grep ERROR

# Export logs
docker-compose logs simple-utcd > utc-logs.txt
```

### Metrics Collection

Integrate with monitoring systems:

```yaml
services:
  simple-utcd:
    labels:
      - "prometheus.scrape=true"
      - "prometheus.port=9090"
```

## Troubleshooting

### Common Issues

1. **Port Already in Use**
   ```bash
   # Check port usage
   sudo lsof -i :37

   # Stop conflicting service
   sudo systemctl stop time
   ```

2. **Permission Issues**
   ```bash
   # Fix directory permissions
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

# Access container shell
docker-compose exec simple-utcd bash
```

### Container Inspection

```bash
# Inspect container details
docker inspect simple-utcd

# Check resource usage
docker stats simple-utcd

# View container processes
docker exec simple-utcd ps aux
```

## Scaling and Load Balancing

### Multiple Instances

```yaml
services:
  simple-utcd:
    deploy:
      replicas: 3
      endpoint_mode: dnsrr
```

### Load Balancer Configuration

Use external load balancers (HAProxy, nginx) to distribute UTC requests:

```yaml
# HAProxy configuration example
backend utc_backend
    balance roundrobin
    server utc1 simple-utcd-1:37 check
    server utc2 simple-utcd-2:37 check
    server utc3 simple-utcd-3:37 check
```

## Backup and Recovery

### Configuration Backup

```bash
# Backup configuration
docker cp simple-utcd:/etc/simple-utcd/simple-utcd.conf ./backup/

# Backup logs
docker cp simple-utcd:/var/log/simple-utcd/ ./backup/logs/
```

### Configuration Restore

```bash
# Restore configuration
docker cp ./backup/simple-utcd.conf simple-utcd:/etc/simple-utcd/
docker-compose restart simple-utcd
```

### Volume Backup

```bash
# Backup volumes
docker run --rm -v simple-utcd_config:/data -v $(pwd):/backup alpine tar czf /backup/config-backup.tar.gz -C /data .
```

## Performance Optimization

### Container Tuning

```yaml
services:
  simple-utcd:
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
# Increase network buffer sizes
echo 'net.core.rmem_max=26214400' >> /etc/sysctl.conf
echo 'net.core.wmem_max=26214400' >> /etc/sysctl.conf
sysctl -p
```

## Security Best Practices

1. **Use non-root user**: The container runs as `simple-utcd` user
2. **Read-only configuration**: Mount config as read-only
3. **Network isolation**: Use custom bridge networks
4. **Resource limits**: Set appropriate CPU and memory limits
5. **Regular updates**: Keep base images updated
6. **Scan images**: Use vulnerability scanners
7. **Secrets management**: Use Docker secrets for sensitive data

## Integration Examples

### Kubernetes

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
        env:
        - name: SIMPLE_UTCD_LOG_LEVEL
          value: "INFO"
```

### Docker Swarm

```yaml
version: '3.8'
services:
  simple-utcd:
    image: simple-utcd:latest
    deploy:
      replicas: 3
      placement:
        constraints:
          - node.role == worker
      resources:
        limits:
          memory: 256M
        reservations:
          memory: 128M
    ports:
      - "37:37"
    networks:
      - utc-network
```

## Support and Maintenance

### Regular Maintenance Tasks

1. **Update base images** monthly
2. **Monitor resource usage** and adjust limits
3. **Rotate logs** to prevent disk space issues
4. **Backup configurations** before updates
5. **Test health checks** regularly

### Getting Help

- Check container logs: `docker-compose logs simple-utcd`
- Validate configuration: `docker-compose exec simple-utcd simple-utcd --test-config`
- Review health status: `docker inspect simple-utcd`
- Check resource usage: `docker stats simple-utcd`
