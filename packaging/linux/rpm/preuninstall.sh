#!/bin/bash
# Pre-uninstallation script for simple-utcd RPM

set -e

PROJECT_NAME="simple-utcd"

# Stop service before removal
if [ "$1" -eq 0 ]; then
    systemctl stop "$simple-utcd" 2>/dev/null || true
    systemctl disable "$simple-utcd" 2>/dev/null || true
fi

exit 0

