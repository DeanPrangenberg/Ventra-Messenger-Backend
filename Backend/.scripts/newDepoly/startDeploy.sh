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
kubectl get storageclass local-storage >/dev/null 2>&1 || {
    log "Creating local storage class..."
    kubectl apply -f "$BACKEND_ROOT_DIR/.config/kubernetes/storageClasses/local-storage.yaml"
    log "Local storage class created."
}
log "Storage class setup complete."

#
# Creating PVCs if not exists
#

log "Creating Persistent Volume Claims (PVCs) if not exists..."
if ! kubectl get pv local-pv-vault-0 >/dev/null 2>&1; then
    log "Creating Persistent Volume for Vault..."
    log_warn "This will create a local storage PV for Vault. Ensure that the local storage directory is set up correctly."
    if [ ! -d /mnt/vault-data-0 ]; then
        sudo mkdir -p /mnt/vault-data-0
        sudo chown 1000:1000 /mnt/vault-data-0
        sudo chmod 700 /mnt/vault-data-0
    fi
    NODE_NAME=$(kubectl get nodes -o jsonpath='{.items[0].metadata.name}')
    sed "s/__NODE_NAME__/$NODE_NAME/g" "$BACKEND_ROOT_DIR/.config/kubernetes/PVCs/local-storage-pvc.yaml" | kubectl apply -f -
    log "Persistent Volume for Vault created."
else
    log "Persistent Volume for Vault already exists. Skipping creation."
fi

#
# Setup Vault
#

log "Setting up Vault..."
chmod +x "$BACKEND_ROOT_DIR/.scripts/newDepoly/installScripts/vault.sh"
"$BACKEND_ROOT_DIR/.scripts/newDepoly/installScripts/vault.sh"

chmod +x "$BACKEND_ROOT_DIR/.scripts/newDepoly/servicesExtern/vault/transit-Vault.sh"
"$BACKEND_ROOT_DIR/.scripts/newDepoly/servicesExtern/vault/transit-Vault.sh"

log_important_user "Encrypting Transit-Vault unseal keys and root token enter your password to encrypt the keys and root token (very important, do not lose it!)"
log_important_user "This Password is used to unseal the Transit-Vault after a server reboot or if the Transit-Vault pod is restarted."
chmod +x "$BACKEND_ROOT_DIR/.scripts/security/toggle-crypt.py"
"$BACKEND_ROOT_DIR/.scripts/security/toggle-crypt.py" "$UNSEAL_TOKEN_TRANSIT_FILE"

log "Vault setup complete"

