#!/bin/bash
set -euo pipefail

#
# Script header
#

# Script specific directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BACKEND_ROOT_DIR="$SCRIPT_DIR/../../../.."
VAULT_CONFIG_DIR=$BACKEND_ROOT_DIR/.config/kubernetes/vault
VAULT_OTHER_DATA_DIR=$BACKEND_ROOT_DIR/.data/other/vault
VAULT_TMP_DATA_DIR=$BACKEND_ROOT_DIR/.data/tmp/vault
AUTO_UNSEAL_TOKEN_FILE=$VAULT_TMP_DATA_DIR/autounseal-token.txt
HOST_IP=$(hostname -I | awk '{print $1}')
TRANSIT_VAULT_NODE_PORT=30201
PKI_VAULT_NODE_PORT=30200

# Source shared functions
source "$BACKEND_ROOT_DIR/scripts/functions/logs.sh"
source "$BACKEND_ROOT_DIR/scripts/functions/env.sh"

#
# Creatie Vault namespace if it doesn't exist
#

log "Creating Vault namespace if it doesn't exist..."
kubectl create namespace vault || log "Vault namespace already exists."
log "Vault namespace is ready."

#
# PKI Vault Installation and inti Configuration
#
log "Creating tmp config files for PKI-Vault..."
mkdir -p "$VAULT_TMP_DATA_DIR"

# Clean the token (remove quotes and newlines)
AUTO_UNSEAL_TOKEN_CLEAN=$(tr -d '"\n' < "$AUTO_UNSEAL_TOKEN_FILE")

# Replace token in values file
sed "s|____autounseal-token.txt____|$AUTO_UNSEAL_TOKEN_CLEAN|g" "$VAULT_CONFIG_DIR/pki-vault-values.yaml" > "$VAULT_TMP_DATA_DIR/pki-vault-values.yaml"
log "Tmp config files for PKI-Vault created."

helm repo add hashicorp https://helm.releases.hashicorp.com
helm repo update
helm upgrade --install pki-vault hashicorp/vault \
  -n vault \
  -f "$VAULT_TMP_DATA_DIR"/pki-vault-values.yaml
log "PKI-Vault installation complete."

kubectl patch svc pki-vault -n vault -p '{"spec": {"type": "NodePort", "ports": [{"name": "http", "port": 8200, "targetPort": 8200, "nodePort": 30200}]}}'
log "PKI-Vault service patched to NodePort."

rm -f "$VAULT_CONFIG_DIR/vault.env"
save_env_var PKI_VAULT_ADDR "http://$HOST_IP:$PKI_VAULT_NODE_PORT" "$VAULT_OTHER_DATA_DIR/vault.env"
log "PKI-Vault data saved to environment file."