#!/bin/sh
set -u

SERVICE_NAME="${SERVICE_NAME:-unknown}"

CERT_PATH="/etc/tls/cert.pem"
KEY_PATH="/etc/tls/key.pem"
CA_PATH="/etc/tls/ca.pem"

log() { echo "[entrypoint][$SERVICE_NAME] $*"; }

TZ="${TZ:-UTC}"
export TZ
log "Timezone set to $TZ"

if command -v ntpd >/dev/null 2>&1; then
  log "Syncing time with ntpd..."
  ntpd -q -p pool.ntp.org || log "ntpd failed, continuing anyway"
else
  log "ntpd not found, skipping time sync"
fi

log "Current date/time: $(date -u)"

for f in "$CERT_PATH" "$KEY_PATH" "$CA_PATH"; do
  while [ ! -s "$f" ]; do
    log "Waiting for $f to exist and be non-empty..."
    sleep 2
  done
done

while :; do
  log "Starting service: /usr/local/bin/$SERVICE_NAME"
  /usr/local/bin/$SERVICE_NAME || log "Service exited with code $?; restarting..."
  sleep 2
done