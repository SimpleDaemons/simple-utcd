#!/bin/bash
# Post-uninstallation script for simple-utcd RPM

set -e

# Reload systemd
systemctl daemon-reload

exit 0

