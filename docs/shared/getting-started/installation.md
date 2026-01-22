# Installation Guide

This guide provides detailed installation instructions for Simple UTC Daemon on various platforms.

## System Requirements

### Minimum Requirements
- **CPU**: 1 core, 1 GHz
- **Memory**: 512 MB RAM
- **Disk**: 100 MB free space
- **Network**: Port 37 (UTC) available

### Recommended Requirements
- **CPU**: 2+ cores, 2+ GHz
- **Memory**: 2+ GB RAM
- **Disk**: 1+ GB free space
- **Network**: Dedicated network interface

### Supported Platforms
- **Linux**: Ubuntu 18.04+, CentOS 7+, RHEL 7+, Debian 9+
- **macOS**: 10.15+ (Catalina and later)
- **Windows**: Windows 10+, Windows Server 2016+

## Installation Methods

### Method 1: Pre-built Packages (Recommended)

#### Linux - Ubuntu/Debian
```bash
# Download and install .deb package
wget https://github.com/simpledaemons/simple-utcd/releases/latest/download/simple-utcd_0.1.0_amd64.deb
sudo dpkg -i simple-utcd_0.1.0_amd64.deb

# Install dependencies if needed
sudo apt-get install -f
```

#### Linux - CentOS/RHEL
```bash
# Download and install .rpm package
wget https://github.com/simpledaemons/simple-utcd/releases/latest/download/simple-utcd-0.1.0-1.x86_64.rpm
sudo rpm -i simple-utcd-0.1.0-1.x86_64.rpm
```

#### macOS
```bash
# Download and install .pkg package
curl -L -O https://github.com/simpledaemons/simple-utcd/releases/latest/download/simple-utcd-0.1.0.pkg
sudo installer -pkg simple-utcd-0.1.0.pkg -target /
```

#### Windows
```cmd
# Download and run .msi installer
# Download from: https://github.com/simpledaemons/simple-utcd/releases/latest/download/simple-utcd-0.1.0.msi
# Run the installer as Administrator
```

### Method 2: Pre-built Binaries

#### Linux/macOS
```bash
# Download binary
wget https://github.com/simpledaemons/simple-utcd/releases/latest/download/simple-utcd-linux-amd64.tar.gz
tar -xzf simple-utcd-linux-amd64.tar.gz

# Install to system
sudo cp simple-utcd /usr/local/bin/
sudo chmod +x /usr/local/bin/simple-utcd

# Create system user
sudo useradd -r -s /bin/false -d /var/lib/simple-utcd simple-utcd

# Create directories
sudo mkdir -p /etc/simple-utcd /var/log/simple-utcd /var/lib/simple-utcd
sudo chown simple-utcd:simple-utcd /var/log/simple-utcd /var/lib/simple-utcd
```

#### Windows
```cmd
# Download binary
# Download from: https://github.com/simpledaemons/simple-utcd/releases/latest/download/simple-utcd-windows-amd64.zip
# Extract to C:\Program Files\Simple-UTCd\

# Create directories
mkdir C:\ProgramData\Simple-UTCd
mkdir C:\ProgramData\Simple-UTCd\logs
mkdir C:\ProgramData\Simple-UTCd\data
```

### Method 3: Build from Source

#### Prerequisites
- **C++17 Compiler**: GCC 7+, Clang 6+, or MSVC 2017+
- **CMake**: 3.12+
- **Git**: For cloning the repository

#### Linux/macOS
```bash
# Install build dependencies
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install build-essential cmake git

# CentOS/RHEL
sudo yum groupinstall "Development Tools"
sudo yum install cmake git

# macOS
brew install cmake git

# Clone and build
git clone https://github.com/simpledaemons/simple-utcd.git
cd simple-utcd
make build

# Install
sudo make install
```

#### Windows
```cmd
# Install Visual Studio 2019+ or Build Tools
# Install CMake
# Install Git

# Clone and build
git clone https://github.com/simpledaemons/simple-utcd.git
cd simple-utcd
mkdir build
cd build
cmake ..
cmake --build . --config Release

# Install (run as Administrator)
cmake --install . --config Release
```

## Post-Installation Setup

### 1. Create Configuration
```bash
# Copy example configuration
sudo cp /usr/share/simple-utcd/config/examples/simple/simple-utcd.conf.example /etc/simple-utcd/simple-utcd.conf

# Edit configuration
sudo nano /etc/simple-utcd/simple-utcd.conf
```

### 2. Create System User (Linux/macOS)
```bash
# Create dedicated user
sudo useradd -r -s /bin/false -d /var/lib/simple-utcd simple-utcd

# Set ownership
sudo chown -R simple-utcd:simple-utcd /var/log/simple-utcd /var/lib/simple-utcd
sudo chown simple-utcd:simple-utcd /etc/simple-utcd/simple-utcd.conf
```

### 3. Configure Firewall
```bash
# Linux - UFW
sudo ufw allow 37/udp
sudo ufw allow 37/tcp

# Linux - firewalld
sudo firewall-cmd --permanent --add-port=37/udp
sudo firewall-cmd --permanent --add-port=37/tcp
sudo firewall-cmd --reload

# macOS
sudo pfctl -f /etc/pf.conf
```

### 4. Enable Service (Linux/macOS)
```bash
# systemd
sudo systemctl enable simple-utcd
sudo systemctl start simple-utcd

# launchd (macOS)
sudo launchctl load /Library/LaunchDaemons/com.simpledaemons.simple-utcd.plist
```

### 5. Windows Service Installation
```cmd
# Run as Administrator
cd "C:\Program Files\Simple-UTCd"
install-service.bat
```

## Verification

### Check Installation
```bash
# Check binary
which simple-utcd
simple-utcd --version

# Check service status (Linux/macOS)
sudo systemctl status simple-utcd

# Check service status (Windows)
sc query Simple-UTCd
```

### Test Functionality
```bash
# Test UTC service
telnet localhost 37

# Check logs
tail -f /var/log/simple-utcd/simple-utcd.log
```

## Uninstallation

### Linux - Package Manager
```bash
# Ubuntu/Debian
sudo apt-get remove simple-utcd

# CentOS/RHEL
sudo yum remove simple-utcd
```

### Linux/macOS - Manual
```bash
# Stop service
sudo systemctl stop simple-utcd
sudo systemctl disable simple-utcd

# Remove files
sudo rm -f /usr/local/bin/simple-utcd
sudo rm -rf /etc/simple-utcd
sudo rm -rf /var/log/simple-utcd
sudo rm -rf /var/lib/simple-utcd
sudo userdel simple-utcd
```

### Windows
```cmd
# Stop and remove service
sc stop Simple-UTCd
sc delete Simple-UTCd

# Remove files
rmdir /s "C:\Program Files\Simple-UTCd"
rmdir /s "C:\ProgramData\Simple-UTCd"
```

## Troubleshooting Installation

### Common Issues

#### Permission Denied
```bash
# Check file permissions
ls -la /usr/local/bin/simple-utcd

# Fix permissions
sudo chmod +x /usr/local/bin/simple-utcd
```

#### Port Already in Use
```bash
# Check what's using port 37
sudo netstat -tulpn | grep :37

# Change port in configuration
listen_port = 1037
```

#### Service Won't Start
```bash
# Check logs
journalctl -u simple-utcd -f

# Run in foreground for debugging
sudo simple-utcd -c /etc/simple-utcd/simple-utcd.conf --debug
```

#### Missing Dependencies
```bash
# Check shared libraries
ldd /usr/local/bin/simple-utcd

# Install missing dependencies
sudo apt-get install libc6 libstdc++6
```

## Next Steps

- **Configuration**: See [Configuration Guide](../configuration/README.md)
- **First Steps**: See [First Steps Guide](first-steps.md)
- **Production Setup**: See [Production Setup](../deployment/production.md)

---

*Next: [First Steps Guide](first-steps.md) to configure and test your installation*
