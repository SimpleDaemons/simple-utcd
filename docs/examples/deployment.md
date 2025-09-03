# Deployment Examples

This guide provides step-by-step deployment examples for Simple UTC Daemon in various environments.

## Single Server Deployment

### Basic Single Server Setup
```bash
#!/bin/bash
# Basic single server deployment

# 1. Install Simple UTC Daemon
sudo apt-get update
sudo apt-get install simple-utcd

# 2. Create configuration
sudo mkdir -p /etc/simple-utcd
sudo cp /usr/share/simple-utcd/config/examples/simple/simple-utcd.conf.example /etc/simple-utcd/simple-utcd.conf

# 3. Create system user
sudo useradd -r -s /bin/false -d /var/lib/simple-utcd simple-utcd

# 4. Set permissions
sudo chown -R simple-utcd:simple-utcd /var/log/simple-utcd /var/lib/simple-utcd
sudo chown simple-utcd:simple-utcd /etc/simple-utcd/simple-utcd.conf

# 5. Configure firewall
sudo ufw allow 37/udp
sudo ufw allow 37/tcp

# 6. Enable and start service
sudo systemctl enable simple-utcd
sudo systemctl start simple-utcd

# 7. Verify deployment
sudo systemctl status simple-utcd
nc -zv localhost 37
```

### Production Single Server Setup
```bash
#!/bin/bash
# Production single server deployment

# 1. Install dependencies
sudo apt-get update
sudo apt-get install -y curl wget gnupg2 software-properties-common

# 2. Add repository and install
wget -qO - https://packages.simpledaemons.com/simple-utcd/gpg.key | sudo apt-key add -
echo "deb https://packages.simpledaemons.com/simple-utcd/ubuntu focal main" | sudo tee /etc/apt/sources.list.d/simple-utcd.list
sudo apt-get update
sudo apt-get install simple-utcd

# 3. Create production configuration
sudo mkdir -p /etc/simple-utcd
sudo tee /etc/simple-utcd/simple-utcd.conf > /dev/null <<EOF
# Production configuration
listen_address = 0.0.0.0
listen_port = 37
enable_ipv6 = true
max_connections = 10000

stratum = 2
sync_interval = 32
timeout = 500

log_level = WARN
enable_console_logging = false
enable_syslog = true
log_file = /var/log/simple-utcd/simple-utcd.log

enable_authentication = true
authentication_key = \${UTCD_AUTH_KEY}
allowed_clients = ["192.168.0.0/16", "10.0.0.0/8"]

worker_threads = 8
enable_statistics = true
stats_interval = 60
EOF

# 4. Set authentication key
sudo tee /etc/simple-utcd/auth-key <<EOF
your-production-secret-key-here
EOF
sudo chmod 600 /etc/simple-utcd/auth-key

# 5. Create system user and directories
sudo useradd -r -s /bin/false -d /var/lib/simple-utcd simple-utcd
sudo mkdir -p /var/log/simple-utcd /var/lib/simple-utcd
sudo chown -R simple-utcd:simple-utcd /var/log/simple-utcd /var/lib/simple-utcd

# 6. Configure logrotate
sudo tee /etc/logrotate.d/simple-utcd > /dev/null <<EOF
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

# 7. Configure firewall
sudo ufw --force enable
sudo ufw allow ssh
sudo ufw allow from 192.168.0.0/16 to any port 37
sudo ufw allow from 10.0.0.0/8 to any port 37

# 8. Enable and start service
sudo systemctl enable simple-utcd
sudo systemctl start simple-utcd

# 9. Verify deployment
sudo systemctl status simple-utcd
nc -zv localhost 37
```

## High Availability Deployment

### Load Balancer Setup (HAProxy)
```bash
#!/bin/bash
# HAProxy load balancer setup

# 1. Install HAProxy
sudo apt-get update
sudo apt-get install -y haproxy

# 2. Configure HAProxy
sudo tee /etc/haproxy/haproxy.cfg > /dev/null <<EOF
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
EOF

# 3. Enable and start HAProxy
sudo systemctl enable haproxy
sudo systemctl start haproxy

# 4. Configure firewall
sudo ufw allow 37/udp
sudo ufw allow 37/tcp
sudo ufw allow 8404/tcp

# 5. Verify HAProxy
sudo systemctl status haproxy
curl http://localhost:8404/stats
```

### Cluster Node Setup
```bash
#!/bin/bash
# Cluster node setup script
NODE_IP=$1
NODE_NAME=$2

if [ -z "$NODE_IP" ] || [ -z "$NODE_NAME" ]; then
    echo "Usage: $0 <node-ip> <node-name>"
    exit 1
fi

# 1. Install Simple UTC Daemon
sudo apt-get update
sudo apt-get install simple-utcd

# 2. Create cluster configuration
sudo mkdir -p /etc/simple-utcd
sudo tee /etc/simple-utcd/simple-utcd.conf > /dev/null <<EOF
# Cluster node configuration
listen_address = $NODE_IP
listen_port = 37
enable_ipv6 = true
max_connections = 5000

stratum = 2
sync_interval = 32
timeout = 500

log_level = INFO
enable_console_logging = false
enable_syslog = true
log_file = /var/log/simple-utcd/simple-utcd.log

enable_authentication = true
authentication_key = \${UTCD_AUTH_KEY}
allowed_clients = ["192.168.0.0/16", "10.0.0.0/8"]

worker_threads = 4
enable_statistics = true
stats_interval = 30

# Cluster configuration
enable_clustering = true
cluster_nodes = ["192.168.1.10:37", "192.168.1.11:37", "192.168.1.12:37"]
cluster_heartbeat_interval = 5
cluster_failover_timeout = 30
EOF

# 3. Set authentication key
sudo tee /etc/simple-utcd/auth-key <<EOF
your-cluster-secret-key-here
EOF
sudo chmod 600 /etc/simple-utcd/auth-key

# 4. Create system user and directories
sudo useradd -r -s /bin/false -d /var/lib/simple-utcd simple-utcd
sudo mkdir -p /var/log/simple-utcd /var/lib/simple-utcd
sudo chown -R simple-utcd:simple-utcd /var/log/simple-utcd /var/lib/simple-utcd

# 5. Configure firewall
sudo ufw allow from 192.168.0.0/16 to any port 37
sudo ufw allow from 10.0.0.0/8 to any port 37

# 6. Enable and start service
sudo systemctl enable simple-utcd
sudo systemctl start simple-utcd

# 7. Verify deployment
sudo systemctl status simple-utcd
nc -zv $NODE_IP 37
```

## Docker Deployment

### Quick Docker Deployment
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

### Docker Compose Setup (Production)
```yaml
# docker-compose.yml
version: '3.8'

services:
  simple-utcd:
    build:
      context: .
      dockerfile: Dockerfile
      target: runtime
    container_name: simple-utcd
    ports:
      - "37:37/tcp"
    volumes:
      - ./config:/etc/simple-utcd:ro
      - ./logs:/var/log/simple-utcd
    environment:
      - SIMPLE_UTCD_LOG_LEVEL=INFO
      - SIMPLE_UTCD_LISTEN_ADDRESS=0.0.0.0
      - SIMPLE_UTCD_LISTEN_PORT=37
    restart: unless-stopped
    healthcheck:
      test: ["CMD", "nc", "-z", "localhost", "37"]
      interval: 30s
      timeout: 10s
      retries: 3
    networks:
      - utc-network

  haproxy:
    image: haproxy:latest
    container_name: utc-haproxy
    ports:
      - "37:37/tcp"
      - "8404:8404"
    volumes:
      - ./haproxy.cfg:/usr/local/etc/haproxy/haproxy.cfg:ro
    depends_on:
      - simple-utcd
    restart: unless-stopped
    networks:
      - utc-network

networks:
  utc-network:
    driver: bridge
```

### Docker Development Environment
```bash
# Start development environment
docker-compose --profile dev up dev

# Access development container
docker-compose exec dev bash

# Build for all platforms
./scripts/build-docker.sh -d all

# Deploy with custom settings
./scripts/deploy-docker.sh -p runtime -c ./config -l ./logs
```

### Docker Compose Deployment Script
```bash
#!/bin/bash
# Docker Compose deployment script

# 1. Create configuration directory
mkdir -p config logs

# 2. Create configuration file
cat > config/simple-utcd.conf <<EOF
listen_address = 0.0.0.0
listen_port = 37
enable_ipv6 = true
max_connections = 1000

stratum = 2
sync_interval = 64
timeout = 1000

log_level = INFO
enable_console_logging = false
log_file = /var/log/simple-utcd/simple-utcd.log

enable_authentication = true
authentication_key = \${UTCD_AUTH_KEY}
allowed_clients = ["192.168.0.0/16", "10.0.0.0/8"]

worker_threads = 4
enable_statistics = true
stats_interval = 60
EOF

# 3. Create HAProxy configuration
cat > haproxy.cfg <<EOF
global
    daemon
    user haproxy
    group haproxy

defaults
    mode tcp
    log global
    option tcplog
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
    server utc1 simple-utcd:37 check

listen stats
    bind *:8404
    stats enable
    stats uri /stats
    stats refresh 30s
EOF

# 4. Start services
docker-compose up -d

# 5. Verify deployment
docker-compose ps
docker-compose logs simple-utcd
nc -zv localhost 37
```

## Kubernetes Deployment

### Kubernetes Manifests
```yaml
# namespace.yaml
apiVersion: v1
kind: Namespace
metadata:
  name: simple-utcd
---
# configmap.yaml
apiVersion: v1
kind: ConfigMap
metadata:
  name: simple-utcd-config
  namespace: simple-utcd
data:
  simple-utcd.conf: |
    listen_address = 0.0.0.0
    listen_port = 37
    enable_ipv6 = true
    max_connections = 1000

    stratum = 2
    sync_interval = 64
    timeout = 1000

    log_level = INFO
    enable_console_logging = false
    log_file = /var/log/simple-utcd/simple-utcd.log

    enable_authentication = true
    authentication_key = ${UTCD_AUTH_KEY}
    allowed_clients = ["192.168.0.0/16", "10.0.0.0/8"]

    worker_threads = 4
    enable_statistics = true
    stats_interval = 60
---
# secret.yaml
apiVersion: v1
kind: Secret
metadata:
  name: simple-utcd-secret
  namespace: simple-utcd
type: Opaque
data:
  auth-key: eW91ci1zZWNyZXQta2V5LWhlcmU=  # base64 encoded
---
# deployment.yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: simple-utcd
  namespace: simple-utcd
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
        env:
        - name: UTCD_AUTH_KEY
          valueFrom:
            secretKeyRef:
              name: simple-utcd-secret
              key: auth-key
        resources:
          requests:
            memory: "256Mi"
            cpu: "250m"
          limits:
            memory: "512Mi"
            cpu: "500m"
        livenessProbe:
          tcpSocket:
            port: 37
          initialDelaySeconds: 30
          periodSeconds: 10
        readinessProbe:
          tcpSocket:
            port: 37
          initialDelaySeconds: 5
          periodSeconds: 5
      volumes:
      - name: config
        configMap:
          name: simple-utcd-config
      - name: logs
        emptyDir: {}
---
# service.yaml
apiVersion: v1
kind: Service
metadata:
  name: simple-utcd-service
  namespace: simple-utcd
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
---
# ingress.yaml
apiVersion: networking.k8s.io/v1
kind: Ingress
metadata:
  name: simple-utcd-ingress
  namespace: simple-utcd
spec:
  rules:
  - host: utc.example.com
    http:
      paths:
      - path: /
        pathType: Prefix
        backend:
          service:
            name: simple-utcd-service
            port:
              number: 37
```

### Kubernetes Deployment Script
```bash
#!/bin/bash
# Kubernetes deployment script

# 1. Create namespace
kubectl apply -f namespace.yaml

# 2. Create configuration
kubectl apply -f configmap.yaml

# 3. Create secret
kubectl apply -f secret.yaml

# 4. Deploy application
kubectl apply -f deployment.yaml

# 5. Create service
kubectl apply -f service.yaml

# 6. Create ingress (optional)
kubectl apply -f ingress.yaml

# 7. Verify deployment
kubectl get pods -n simple-utcd
kubectl get services -n simple-utcd
kubectl get ingress -n simple-utcd

# 8. Test connectivity
kubectl port-forward -n simple-utcd service/simple-utcd-service 37:37 &
nc -zv localhost 37
```

## Cloud Deployment

### AWS EC2 Deployment
```bash
#!/bin/bash
# AWS EC2 deployment script

# 1. Launch EC2 instance
aws ec2 run-instances \
    --image-id ami-0c02fb55956c7d316 \
    --instance-type t3.medium \
    --key-name your-key-pair \
    --security-group-ids sg-12345678 \
    --subnet-id subnet-12345678 \
    --tag-specifications 'ResourceType=instance,Tags=[{Key=Name,Value=simple-utcd}]'

# 2. Wait for instance to be running
aws ec2 wait instance-running --instance-ids i-1234567890abcdef0

# 3. Get public IP
PUBLIC_IP=$(aws ec2 describe-instances \
    --instance-ids i-1234567890abcdef0 \
    --query 'Reservations[0].Instances[0].PublicIpAddress' \
    --output text)

# 4. SSH to instance and deploy
ssh -i your-key-pair.pem ubuntu@$PUBLIC_IP << 'EOF'
# Install Simple UTC Daemon
sudo apt-get update
sudo apt-get install -y simple-utcd

# Configure service
sudo systemctl enable simple-utcd
sudo systemctl start simple-utcd

# Verify deployment
sudo systemctl status simple-utcd
EOF

# 5. Test connectivity
nc -zv $PUBLIC_IP 37
```

### Azure VM Deployment
```bash
#!/bin/bash
# Azure VM deployment script

# 1. Create resource group
az group create --name simple-utcd-rg --location eastus

# 2. Create virtual network
az network vnet create \
    --resource-group simple-utcd-rg \
    --name simple-utcd-vnet \
    --address-prefix 10.0.0.0/16 \
    --subnet-name simple-utcd-subnet \
    --subnet-prefix 10.0.1.0/24

# 3. Create network security group
az network nsg create \
    --resource-group simple-utcd-rg \
    --name simple-utcd-nsg

# 4. Add security rule
az network nsg rule create \
    --resource-group simple-utcd-rg \
    --nsg-name simple-utcd-nsg \
    --name allow-utc \
    --priority 1000 \
    --source-address-prefixes '*' \
    --source-port-ranges '*' \
    --destination-address-prefixes '*' \
    --destination-port-ranges 37 \
    --access Allow \
    --protocol '*'

# 5. Create public IP
az network public-ip create \
    --resource-group simple-utcd-rg \
    --name simple-utcd-pip \
    --allocation-method Static

# 6. Create network interface
az network nic create \
    --resource-group simple-utcd-rg \
    --name simple-utcd-nic \
    --vnet-name simple-utcd-vnet \
    --subnet simple-utcd-subnet \
    --public-ip-address simple-utcd-pip \
    --network-security-group simple-utcd-nsg

# 7. Create virtual machine
az vm create \
    --resource-group simple-utcd-rg \
    --name simple-utcd-vm \
    --image UbuntuLTS \
    --size Standard_B2s \
    --admin-username azureuser \
    --generate-ssh-keys \
    --nics simple-utcd-nic

# 8. Get public IP
PUBLIC_IP=$(az vm show \
    --resource-group simple-utcd-rg \
    --name simple-utcd-vm \
    --show-details \
    --query publicIps \
    --output tsv)

# 9. SSH to VM and deploy
ssh azureuser@$PUBLIC_IP << 'EOF'
# Install Simple UTC Daemon
sudo apt-get update
sudo apt-get install -y simple-utcd

# Configure service
sudo systemctl enable simple-utcd
sudo systemctl start simple-utcd

# Verify deployment
sudo systemctl status simple-utcd
EOF

# 10. Test connectivity
nc -zv $PUBLIC_IP 37
```

## Verification and Testing

### Deployment Verification Script
```bash
#!/bin/bash
# Deployment verification script

HOST=${1:-localhost}
PORT=${2:-37}

echo "🔍 Verifying Simple UTC Daemon deployment on $HOST:$PORT"

# Check if service is running
if nc -z $HOST $PORT; then
    echo "✅ Service is listening on $HOST:$PORT"
else
    echo "❌ Service is not listening on $HOST:$PORT"
    exit 1
fi

# Test UTC time request
echo "🕐 Testing UTC time request..."
if timeout 5 bash -c "echo '' | nc $HOST $PORT" > /dev/null; then
    echo "✅ UTC time request successful"
else
    echo "❌ UTC time request failed"
    exit 1
fi

# Test multiple connections
echo "🔗 Testing multiple connections..."
for i in {1..5}; do
    if timeout 5 bash -c "echo '' | nc $HOST $PORT" > /dev/null; then
        echo "✅ Connection $i successful"
    else
        echo "❌ Connection $i failed"
        exit 1
    fi
done

echo "🎉 Deployment verification successful!"
```

## Next Steps

- **Docker Deployment**: See [Docker Deployment Guide](../deployment/docker.md) for comprehensive Docker setup
- **Monitoring**: Set up monitoring and alerting
- **Backup**: Configure backup and recovery
- **Security**: Implement security hardening
- **Performance**: Optimize for your workload

---

*These deployment examples provide comprehensive guidance for deploying Simple UTC Daemon in various environments.*
