#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BACKEND_ROOT_DIR="$SCRIPT_DIR/../.."
echo "[DEBUG] Script directory is: $SCRIPT_DIR"
echo "[DEBUG] Backend root directory is: $BACKEND_ROOT_DIR"

source "$BACKEND_ROOT_DIR/.scripts/functions/logs.sh"
source "$BACKEND_ROOT_DIR/.scripts/functions/env.sh"

# K3s stoppen
log "Stopping k3s..."
sudo systemctl stop k3s || { error "Failed to stop k3s"; exit 1; }

# Warten, bis k3s vollständig gestoppt ist
log "Waiting for K3s to stop..."
while systemctl is-active --quiet k3s; do
  sleep 1
done
log "K3s stopped successfully."

# K3s Datenverzeichnis löschen (ACHTUNG: Löscht alle Daten!)
log "Deleting K3s data directory..."
sudo rm -rf /var/lib/rancher/k3s/server/db/ || { error "Failed to delete K3s data directory"; exit 1; }

# K3s starten
log "Starting k3s..."
sudo systemctl start k3s || { error "Failed to start k3s"; exit 1; }

# Warten, bis k3s wieder aktiv ist
log "Waiting for K3s to become active..."
while ! systemctl is-active --quiet k3s; do
  sleep 1
done
log "K3s is now active."

# Warten, bis kubectl funktioniert (dauert etwas nach Start)
log "Waiting for kubectl to be ready..."
while ! kubectl get ns >/dev/null 2>&1; do
  sleep 1
done

# Letzte Prüfung
log "Checking K3s status..."
kubectl get ns

log "Script completed successfully."