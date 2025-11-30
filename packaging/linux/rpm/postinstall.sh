#!/bin/bash
# Post-installation script for simple-utcd RPM

set -e

PROJECT_NAME="simple-utcd"
SERVICE_USER="utcddev"

# Create service user if it doesn't exist
if ! id "$SERVICE_USER" &>/dev/null; then
    useradd -r -s /sbin/nologin -d /var/lib/$simple-utcd -c "$simple-utcd service user" "$SERVICE_USER"
fi

# Set ownership
chown -R "$SERVICE_USER:$SERVICE_USER" /etc/$simple-utcd 2>/dev/null || true
chown -R "$SERVICE_USER:$SERVICE_USER" /var/log/$simple-utcd 2>/dev/null || true
chown -R "$SERVICE_USER:$SERVICE_USER" /var/lib/$simple-utcd 2>/dev/null || true

# Enable and start service
systemctl daemon-reload
systemctl enable "$simple-utcd" 2>/dev/null || true
systemctl start "$simple-utcd" 2>/dev/null || true

exit 0

