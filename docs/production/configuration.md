# Production Version Configuration Guide

**Version:** 0.3.2  
**License:** Apache 2.0

---

## Overview

This guide covers configuration for the Production Version of Simple UTC Daemon. The Production Version supports JSON, YAML, and INI configuration formats.

## Configuration File Location

Default configuration file locations:
- **Linux:** `/etc/simple-utcd/simple-utcd.conf`
- **macOS:** `/usr/local/etc/simple-utcd/simple-utcd.conf`
- **Windows:** `C:\Program Files\Simple UTC Daemon\config\simple-utcd.conf`

## Basic Configuration

### Minimal Configuration

```json
{
  "utc": {
    "listen_address": "0.0.0.0",
    "port": 37
  }
}
```

### Complete Configuration Example

```json
{
  "utc": {
    "listen_address": "0.0.0.0",
    "port": 37,
    "authentication": {
      "enabled": true,
      "method": "sha256"
    },
    "acl": {
      "enabled": true,
      "allowed_clients": ["192.168.1.0/24"]
    },
    "logging": {
      "level": "INFO",
      "file": "/var/log/simple-utcd/simple-utcd.log"
    }
  }
}
```

## Configuration Reference

See [Configuration Reference](../shared/configuration/README.md) for complete options.

## Hot Reloading

The Production Version supports hot reloading via SIGHUP:

```bash
sudo killall -HUP simple-utcd
```

---

**Last Updated:** December 2024  
**Version:** 0.3.2
