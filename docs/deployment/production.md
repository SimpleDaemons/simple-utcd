# Production Setup Guide

This guide covers enterprise-grade production deployment of Simple UTC Daemon with high availability, security, and monitoring.

## Production Architecture

### Recommended Architecture
```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   Load Balancer │    │   Load Balancer │    │   Load Balancer │
│   (HAProxy)     │    │   (Nginx)       │    │   (Cloud LB)    │
└─────────┬───────┘    └─────────┬───────┘    └─────────┬───────┘
          │                      │                      │
          └──────────────────────┼──────────────────────┘
                                 │
          ┌──────────────────────┼──────────────────────┐
          │                      │                      │
┌─────────▼───────┐    ┌─────────▼───────┐    ┌─────────▼───────┐
│  UTC Server 1   │    │  UTC Server 2   │    │  UTC Server 3   │
│  (Primary)      │    │  (Secondary)    │    │  (Tertiary)     │
└─────────────────┘    └─────────────────┘    └─────────────────┘
          │                      │                      │
          └──────────────────────┼──────────────────────┘
                                 │
          ┌──────────────────────┼──────────────────────┐
          │                      │                      │
┌─────────▼───────┐    ┌─────────▼───────┐    ┌─────────▼───────┐
│   Monitoring    │    │   Logging       │    │   Backup        │
│   (Prometheus)  │    │   (ELK Stack)   │    │   (Automated)   │
└─────────────────┘    └─────────────────┘    └─────────────────┘
```

## Production Requirements

### Hardware Requirements
- **CPU**: 4+ cores, 2.5+ GHz
- **Memory**: 8+ GB RAM
- **Storage**: 100+ GB SSD
- **Network**: 1+ Gbps dedicated connection

### Software Requirements
- **OS**: Ubuntu 20.04 LTS, CentOS 8+, RHEL 8+
- **Kernel**: 5.4+ (Linux)
- **Dependencies**: Latest stable versions

## Production Configuration

### Master Configuration Template
```ini
# Production UTC Daemon Configuration
# File: /etc/simple-utcd/simple-utcd.conf

# Network Configuration
listen_address = 0.0.0.0
listen_port = 37
enable_ipv6 = true
max_connections = 10000

# UTC Server Configuration
stratum = 2
sync_interval = 32
timeout = 500

# Logging Configuration
log_file = /var/log/simple-utcd/simple-utcd.log
log_level = WARN
enable_console_logging = false
enable_syslog = true
log_rotation = true
log_max_size = 100MB
log_max_files = 10

# Security Configuration
enable_authentication = true
authentication_key = ${UTCD_AUTH_KEY}
allowed_clients = ["192.168.0.0/16", "10.0.0.0/8", "172.16.0.0/12"]
denied_clients = []

# Performance Configuration
worker_threads = 16
enable_statistics = true
stats_interval = 30

# Monitoring Configuration
enable_health_check = true
health_check_interval = 10
enable_metrics = true
metrics_port = 8080

# High Availability Configuration
enable_clustering = true
cluster_nodes = ["192.168.1.10:37", "192.168.1.11:37", "192.168.1.12:37"]
cluster_heartbeat_interval = 5
cluster_failover_timeout = 30
```

## High Availability Setup

### Load Balancer Configuration (HAProxy)
```haproxy
# /etc/haproxy/haproxy.cfg
global
    daemon
    user haproxy
    group haproxy
    log 127.0.0.1:514 local0
    chroot /var/lib/haproxy
    stats socket /run/haproxy/admin.sock mode 660 level admin
    stats timeout 30s

defaults
    mode tcp
    log global
    option tcplog
    option dontlognull
    option redispatch
    retries 3
    timeout connect 5000
    timeout client 50000
    timeout server 50000

frontend utc_frontend
    bind *:37
    default_backend utc_servers

backend utc_servers
    balance roundrobin
    option tcp-check
    tcp-check connect port 37
    server utc1 192.168.1.10:37 check
    server utc2 192.168.1.11:37 check
    server utc3 192.168.1.12:37 check

listen stats
    bind *:8404
    stats enable
    stats uri /stats
    stats refresh 30s
    stats admin if TRUE
```

### Load Balancer Configuration (Nginx)
```nginx
# /etc/nginx/nginx.conf
events {
    worker_connections 1024;
}

stream {
    upstream utc_servers {
        server 192.168.1.10:37;
        server 192.168.1.11:37;
        server 192.168.1.12:37;
    }

    server {
        listen 37;
        proxy_pass utc_servers;
        proxy_timeout 1s;
        proxy_responses 1;
    }
}
```

### Cluster Configuration
```bash
#!/bin/bash
# Cluster management script
CLUSTER_NODES=("192.168.1.10" "192.168.1.11" "192.168.1.12")
CLUSTER_PORT=37

check_cluster_health() {
    local healthy_nodes=()
    
    for node in "${CLUSTER_NODES[@]}"; do
        if nc -z $node $CLUSTER_PORT; then
            healthy_nodes+=($node)
            echo "✅ $node is healthy"
        else
            echo "❌ $node is unhealthy"
        fi
    done
    
    echo "Healthy nodes: ${#healthy_nodes[@]}/${#CLUSTER_NODES[@]}"
    return ${#healthy_nodes[@]}
}

failover_cluster() {
    local primary_node=$1
    local backup_node=$2
    
    echo "🔄 Initiating failover from $primary_node to $backup_node"
    
    # Update load balancer configuration
    # (Implementation depends on your load balancer)
    
    # Notify monitoring systems
    curl -X POST "http://monitoring:9093/api/v1/alerts" \
        -H "Content-Type: application/json" \
        -d '{"alerts":[{"labels":{"alertname":"UTCFailover","node":"'$primary_node'"},"annotations":{"description":"UTC server failover initiated"}}]}'
}
```

## Security Hardening

### System Hardening
```bash
#!/bin/bash
# System hardening script

# Disable unnecessary services
systemctl disable bluetooth
systemctl disable cups
systemctl disable avahi-daemon

# Configure firewall
ufw --force enable
ufw default deny incoming
ufw default allow outgoing
ufw allow ssh
ufw allow from 192.168.0.0/16 to any port 37
ufw allow from 10.0.0.0/8 to any port 37

# Configure fail2ban
cat > /etc/fail2ban/jail.d/utc.conf << EOF
[utc]
enabled = true
port = 37
filter = utc
logpath = /var/log/simple-utcd/simple-utcd.log
maxretry = 5
bantime = 3600
findtime = 600
EOF

# Configure logrotate
cat > /etc/logrotate.d/simple-utcd << EOF
/var/log/simple-utcd/*.log {
    daily
    missingok
    rotate 30
    compress
    delaycompress
    notifempty
    create 644 simple-utcd simple-utcd
    postrotate
        systemctl reload simple-utcd
    endscript
}
EOF
```

### Application Security
```ini
# Security-focused configuration
enable_authentication = true
authentication_key = ${UTCD_AUTH_KEY}
allowed_clients = ["192.168.0.0/16", "10.0.0.0/8"]
denied_clients = ["192.168.1.100"]

# Rate limiting
enable_rate_limiting = true
rate_limit_requests = 1000
rate_limit_window = 60

# Connection limits
max_connections_per_ip = 10
max_connections_total = 10000

# Timeout settings
connection_timeout = 5000
idle_timeout = 30000
```

## Monitoring and Alerting

### Prometheus Configuration
```yaml
# /etc/prometheus/prometheus.yml
global:
  scrape_interval: 15s
  evaluation_interval: 15s

rule_files:
  - "utc_rules.yml"

scrape_configs:
  - job_name: 'simple-utcd'
    static_configs:
      - targets: ['192.168.1.10:8080', '192.168.1.11:8080', '192.168.1.12:8080']
    scrape_interval: 5s
    metrics_path: /metrics

alerting:
  alertmanagers:
    - static_configs:
        - targets:
          - alertmanager:9093
```

### Alert Rules
```yaml
# /etc/prometheus/utc_rules.yml
groups:
- name: utc_alerts
  rules:
  - alert: UTCDown
    expr: up{job="simple-utcd"} == 0
    for: 1m
    labels:
      severity: critical
    annotations:
      summary: "UTC server is down"
      description: "UTC server {{ $labels.instance }} has been down for more than 1 minute"

  - alert: UTCHighConnections
    expr: utc_connections_total > 8000
    for: 2m
    labels:
      severity: warning
    annotations:
      summary: "High connection count"
      description: "UTC server {{ $labels.instance }} has {{ $value }} connections"

  - alert: UTCHighErrorRate
    expr: rate(utc_errors_total[5m]) > 0.1
    for: 1m
    labels:
      severity: warning
    annotations:
      summary: "High error rate"
      description: "UTC server {{ $labels.instance }} has error rate {{ $value }}"
```

### Grafana Dashboard
```json
{
  "dashboard": {
    "title": "Simple UTC Daemon",
    "panels": [
      {
        "title": "Connection Count",
        "type": "graph",
        "targets": [
          {
            "expr": "utc_connections_total",
            "legendFormat": "{{instance}}"
          }
        ]
      },
      {
        "title": "Error Rate",
        "type": "graph",
        "targets": [
          {
            "expr": "rate(utc_errors_total[5m])",
            "legendFormat": "{{instance}}"
          }
        ]
      },
      {
        "title": "Response Time",
        "type": "graph",
        "targets": [
          {
            "expr": "utc_response_time_seconds",
            "legendFormat": "{{instance}}"
          }
        ]
      }
    ]
  }
}
```

## Logging and Centralized Log Management

### ELK Stack Configuration
```yaml
# /etc/logstash/conf.d/utc.conf
input {
  file {
    path => "/var/log/simple-utcd/*.log"
    type => "simple-utcd"
    codec => "json"
  }
}

filter {
  if [type] == "simple-utcd" {
    grok {
      match => { "message" => "\[%{TIMESTAMP_ISO8601:timestamp}\] \[%{WORD:level}\] %{GREEDYDATA:message}" }
    }
    
    date {
      match => [ "timestamp", "yyyy-MM-dd HH:mm:ss.SSS" ]
    }
    
    if [level] == "ERROR" {
      mutate {
        add_tag => [ "error" ]
      }
    }
  }
}

output {
  elasticsearch {
    hosts => ["elasticsearch:9200"]
    index => "simple-utcd-%{+YYYY.MM.dd}"
  }
}
```

### Log Analysis Queries
```bash
# Common log analysis queries
# Error rate by hour
curl -X GET "elasticsearch:9200/simple-utcd-*/_search" -H 'Content-Type: application/json' -d'
{
  "aggs": {
    "errors_by_hour": {
      "date_histogram": {
        "field": "@timestamp",
        "interval": "hour"
      },
      "aggs": {
        "error_count": {
          "filter": {
            "term": { "level": "ERROR" }
          }
        }
      }
    }
  }
}'

# Top error messages
curl -X GET "elasticsearch:9200/simple-utcd-*/_search" -H 'Content-Type: application/json' -d'
{
  "aggs": {
    "top_errors": {
      "terms": {
        "field": "message.keyword",
        "size": 10
      }
    }
  }
}'
```

## Backup and Disaster Recovery

### Automated Backup Script
```bash
#!/bin/bash
# Production backup script
BACKUP_DIR="/var/backups/simple-utcd"
TIMESTAMP=$(date +%Y%m%d_%H%M%S)
RETENTION_DAYS=30

# Create backup directory
mkdir -p $BACKUP_DIR

# Backup configuration
cp /etc/simple-utcd/simple-utcd.conf $BACKUP_DIR/config_$TIMESTAMP.conf

# Backup logs
tar -czf $BACKUP_DIR/logs_$TIMESTAMP.tar.gz /var/log/simple-utcd/

# Backup statistics
cp -r /var/lib/simple-utcd/ $BACKUP_DIR/stats_$TIMESTAMP/

# Upload to remote storage (S3, etc.)
aws s3 cp $BACKUP_DIR/config_$TIMESTAMP.conf s3://utc-backups/config/
aws s3 cp $BACKUP_DIR/logs_$TIMESTAMP.tar.gz s3://utc-backups/logs/
aws s3 cp $BACKUP_DIR/stats_$TIMESTAMP/ s3://utc-backups/stats/ --recursive

# Cleanup old backups
find $BACKUP_DIR -name "*.conf" -mtime +$RETENTION_DAYS -delete
find $BACKUP_DIR -name "*.tar.gz" -mtime +$RETENTION_DAYS -delete
find $BACKUP_DIR -name "stats_*" -mtime +$RETENTION_DAYS -exec rm -rf {} \;
```

### Disaster Recovery Plan
```bash
#!/bin/bash
# Disaster recovery script
RECOVERY_NODE=$1
BACKUP_S3_BUCKET="utc-backups"

if [ -z "$RECOVERY_NODE" ]; then
    echo "Usage: $0 <recovery-node-ip>"
    exit 1
fi

echo "🚨 Starting disaster recovery for node $RECOVERY_NODE"

# Download latest backup
aws s3 cp s3://$BACKUP_S3_BUCKET/config/ $RECOVERY_NODE:/etc/simple-utcd/ --recursive
aws s3 cp s3://$BACKUP_S3_BUCKET/logs/ $RECOVERY_NODE:/var/log/simple-utcd/ --recursive
aws s3 cp s3://$BACKUP_S3_BUCKET/stats/ $RECOVERY_NODE:/var/lib/simple-utcd/ --recursive

# Restore service
ssh $RECOVERY_NODE "systemctl start simple-utcd"

# Verify recovery
sleep 10
if ssh $RECOVERY_NODE "systemctl is-active --quiet simple-utcd"; then
    echo "✅ Recovery successful for $RECOVERY_NODE"
else
    echo "❌ Recovery failed for $RECOVERY_NODE"
    exit 1
fi
```

## Performance Optimization

### System Tuning
```bash
#!/bin/bash
# Production system tuning

# Network tuning
echo "net.core.rmem_max = 134217728" >> /etc/sysctl.conf
echo "net.core.wmem_max = 134217728" >> /etc/sysctl.conf
echo "net.ipv4.udp_mem = 1024 87380 134217728" >> /etc/sysctl.conf
echo "net.ipv4.tcp_rmem = 4096 87380 134217728" >> /etc/sysctl.conf
echo "net.ipv4.tcp_wmem = 4096 65536 134217728" >> /etc/sysctl.conf

# File descriptor limits
echo "simple-utcd soft nofile 65536" >> /etc/security/limits.conf
echo "simple-utcd hard nofile 65536" >> /etc/security/limits.conf

# CPU governor
echo performance > /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor

# Apply changes
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

# Memory optimization
enable_memory_pool = true
memory_pool_size = 1024MB
connection_pool_size = 1000
```

## Maintenance Procedures

### Rolling Updates
```bash
#!/bin/bash
# Rolling update script
CLUSTER_NODES=("192.168.1.10" "192.168.1.11" "192.168.1.12")
NEW_VERSION="0.2.0"

for node in "${CLUSTER_NODES[@]}"; do
    echo "🔄 Updating $node to version $NEW_VERSION"
    
    # Drain connections
    ssh $node "systemctl stop simple-utcd"
    
    # Update binary
    scp simple-utcd-$NEW_VERSION $node:/usr/local/bin/simple-utcd
    
    # Start service
    ssh $node "systemctl start simple-utcd"
    
    # Wait for health check
    sleep 30
    if ssh $node "nc -z localhost 37"; then
        echo "✅ $node updated successfully"
    else
        echo "❌ $node update failed"
        exit 1
    fi
done
```

### Health Checks
```bash
#!/bin/bash
# Comprehensive health check
check_production_health() {
    local node=$1
    local issues=()
    
    # Check service status
    if ! ssh $node "systemctl is-active --quiet simple-utcd"; then
        issues+=("Service not running")
    fi
    
    # Check port listening
    if ! ssh $node "nc -z localhost 37"; then
        issues+=("Port 37 not listening")
    fi
    
    # Check memory usage
    local memory=$(ssh $node "ps -o rss= -p \$(pgrep simple-utcd)")
    if [ $memory -gt 1000000 ]; then  # 1GB
        issues+=("High memory usage: ${memory}KB")
    fi
    
    # Check disk space
    local disk=$(ssh $node "df /var/log/simple-utcd | tail -1 | awk '{print \$5}' | sed 's/%//'")
    if [ $disk -gt 80 ]; then
        issues+=("High disk usage: ${disk}%")
    fi
    
    if [ ${#issues[@]} -eq 0 ]; then
        echo "✅ $node is healthy"
        return 0
    else
        echo "❌ $node has issues: ${issues[*]}"
        return 1
    fi
}
```

## Next Steps

- **Examples**: See [Deployment Examples](../examples/deployment.md)
- **Troubleshooting**: See [Troubleshooting Guide](../troubleshooting/README.md)
- **Monitoring**: See [Monitoring Setup](../examples/monitoring/README.md)

---

*This production setup provides enterprise-grade reliability, security, and monitoring for Simple UTC Daemon.*
