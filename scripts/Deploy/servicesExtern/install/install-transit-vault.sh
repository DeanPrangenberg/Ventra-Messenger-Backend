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
# Transit Vault Installation and inti Configuration
#

log "Creating or updating Transit-Vault configMap..."
kubectl create configmap vault-transit-config \
  --from-file=vault-transit-config.hcl="$VAULT_CONFIG_DIR/vault-transit-config.hcl" \
  -n vault \
  --dry-run=client -o yaml | kubectl apply -f -
log "Transit-Vault configMap ready."

log "Installing Transit-Vault via helm..."
helm repo add hashicorp https://helm.releases.hashicorp.com
helm repo update
helm upgrade --install transit-vault hashicorp/vault \
  -n vault \
  -f "$VAULT_CONFIG_DIR"/transit-vault-values.yaml
log "Transit-Vault installation complete."

log "Patching Transit-Vault service to NodePort..."
kubectl patch svc transit-vault -n vault -p '{"spec": {"type": "NodePort", "ports": [{"name": "http", "port": 8200, "targetPort": 8200, "nodePort": 30201}]}}'
log "Transit-Vault service patched to NodePort."

log "Saving Transit-Vault data to environment file..."
rm -f "$VAULT_CONFIG_DIR/vault.env"
save_env_var TRANSIT_VAULT_ADDR "http://$HOST_IP:$TRANSIT_VAULT_NODE_PORT" "$VAULT_OTHER_DATA_DIR/vault.env"
log "Transit-Vault data saved to environment file."
