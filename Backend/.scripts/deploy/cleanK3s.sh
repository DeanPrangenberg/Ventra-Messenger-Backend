#!/bin/bash

# Farben für bessere Lesbarkeit (optional)
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

# Funktion zum Ausgeben von Statusmeldungen
log() {
  echo -e "${GREEN}[INFO]${NC} $1"
}

error() {
  echo -e "${RED}[ERROR]${NC} $1" >&2
}

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