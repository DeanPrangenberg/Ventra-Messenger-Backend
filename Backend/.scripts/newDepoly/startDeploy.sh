#!/bin/bash

set -euo pipefail

#
# Setting up paths and loading functions
#

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BACKEND_ROOT_DIR="$SCRIPT_DIR/../.."
VAULT_OTHER_DATA_DIR=$BACKEND_ROOT_DIR/.data/other/vault
UNSEAL_TOKEN_TRANSIT_FILE=$VAULT_OTHER_DATA_DIR/transit-unseal.json
HOST_IP=$(hostname -I | awk '{print $1}')

source "$BACKEND_ROOT_DIR/.scripts/functions/logs.sh"
source "$BACKEND_ROOT_DIR/.scripts/functions/env.sh"

#
# Installing required CLI tools
#

log "Installing required CLI tools..."
chmod +x "$BACKEND_ROOT_DIR/.scripts/newDepoly/nodeSetup/getCLITools.sh"
"$BACKEND_ROOT_DIR/.scripts/newDepoly/nodeSetup/getCLITools.sh"
log "Required CLI tools installed."

log "Installing Python dependencies..."
chmod +x "$BACKEND_ROOT_DIR/.scripts/newDepoly/nodeSetup/getPipLibs.sh"
"$BACKEND_ROOT_DIR/.scripts/newDepoly/nodeSetup/getPipLibs.sh"
log "Python dependencies installed."

#
# Setup storage class if not exists
#

log "Setting up storage class if not exists..."

kubectl get storageclass local-pki-storage >/dev/null 2>&1 || {
    log "Creating local storage class..."
    kubectl apply -f "$BACKEND_ROOT_DIR/.config/kubernetes/storageClasses/vault/local-pki-storage.yaml"
    log "Local storage class created."
}

kubectl get storageclass local-transit-storage >/dev/null 2>&1 || {
    log "Creating local storage class..."
    kubectl apply -f "$BACKEND_ROOT_DIR/.config/kubernetes/storageClasses/vault/local-transit-storage.yaml"
    log "Local storage class created."
}

log "Storage class setup complete."

#
# Creating PVCs if not exists
#

log "Creating Persistent Volumes (PVs) and Persistent Volume Claims (PVCs) if not exists..."

# PKI
if ! kubectl get pv local-pv-vault-pki-0 >/dev/null 2>&1; then
    log "Creating Persistent Volume for Vault PKI..."
    log_important_user "This will create a local storage PV for Vault PKI. Ensure that the local storage directory is set up correctly."
    if [ ! -d /mnt/vault-pki-data-0 ]; then
        sudo mkdir -p /mnt/vault-pki-data-0
        sudo chown 1000:1000 /mnt/vault-pki-data-0
        sudo chmod 700 /mnt/vault-pki-data-0
    fi
    NODE_NAME=$(kubectl get nodes -o jsonpath='{.items[0].metadata.name}')
    sed "s/__NODE_NAME__/$NODE_NAME/g" "$BACKEND_ROOT_DIR/.config/kubernetes/PVCs/vault/local-pki-storage-pvc.yaml" | kubectl apply -f -
    log "Persistent Volume for Vault PKI created."
else
    log "Persistent Volume for Vault PKI already exists. Skipping creation."
fi

# Transit
if ! kubectl get pv local-pv-vault-transit-0 >/dev/null 2>&1; then
    log "Creating Persistent Volume for Vault Transit..."
    log_important_user "This will create a local storage PV for Vault Transit. Ensure that the local storage directory is set up correctly."
    if [ ! -d /mnt/vault-transit-data-0 ]; then
        sudo mkdir -p /mnt/vault-transit-data-0
        sudo chown 1000:1000 /mnt/vault-transit-data-0
        sudo chmod 700 /mnt/vault-transit-data-0
    fi
    NODE_NAME=$(kubectl get nodes -o jsonpath='{.items[0].metadata.name}')
    sed "s/__NODE_NAME__/$NODE_NAME/g" "$BACKEND_ROOT_DIR/.config/kubernetes/PVCs/vault/local-transit-storage-pvc.yaml" | kubectl apply -f -
    log "Persistent Volume for Vault Transit created."
else
    log "Persistent Volume for Vault Transit already exists. Skipping creation."
fi

log "Persistent Volumes (PVs) and Persistent Volume Claims (PVCs) setup complete."

#
# Setup Vault
#

log "Setting up Vaults..."

# Install and configure Transit Vault first
"$BACKEND_ROOT_DIR/.scripts/newDepoly/servicesExtern/install/install-transit-vault.sh"
chmod +x "$BACKEND_ROOT_DIR/.scripts/newDepoly/servicesExtern/config/config-transit-Vault.sh"
"$BACKEND_ROOT_DIR/.scripts/newDepoly/servicesExtern/config/config-transit-Vault.sh"

log_important_user "Encrypting Transit-Vault unseal keys and root token enter your password to encrypt the keys and root token (very important, do not lose it!)"
chmod +x "$BACKEND_ROOT_DIR/.scripts/security/toggle-crypt.py"
"$BACKEND_ROOT_DIR/.scripts/security/toggle-crypt.py" "$UNSEAL_TOKEN_TRANSIT_FILE"
log "Transit-Vault unseal keys and root token encrypted."

# Now install and configure PKI Vault with auto-unseal
chmod +x "$BACKEND_ROOT_DIR/.scripts/newDepoly/servicesExtern/install/install-pki-vault.sh"
"$BACKEND_ROOT_DIR/.scripts/newDepoly/servicesExtern/install/install-pki-vault.sh"

chmod +x "$BACKEND_ROOT_DIR/.scripts/newDepoly/servicesExtern/config/config-pki-Vault.sh"
"$BACKEND_ROOT_DIR/.scripts/newDepoly/servicesExtern/config/config-pki-Vault.sh"

log "Vaults setup complete"

