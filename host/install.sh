#!/bin/bash
set -e

HOST_DIR="$(cd "$(dirname "$0")" && pwd)"
PLIST_SRC="$HOST_DIR/com.claude.dashboard.plist"
PLIST_DST="$HOME/Library/LaunchAgents/com.claude.dashboard.plist"
PYTHON3="$(which python3)"

echo "Host dir:  $HOST_DIR"
echo "Python3:   $PYTHON3"
echo "Plist:     $PLIST_DST"

# Unload existing if present
launchctl unload "$PLIST_DST" 2>/dev/null || true

# Generate plist with resolved paths
sed -e "s|__HOST_DIR__|$HOST_DIR|g" \
    -e "s|__PYTHON3__|$PYTHON3|g" \
    "$PLIST_SRC" > "$PLIST_DST"

launchctl load "$PLIST_DST"
echo "Installed and started com.claude.dashboard"
