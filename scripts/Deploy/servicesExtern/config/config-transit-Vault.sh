#!/bin/bash
set -euo pipefail

#
# Script header
#

# Script specific directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BACKEND_ROOT_DIR="$SCRIPT_DIR/../../../.."
VAULT_CONFIG_DIR=$BACKEND_ROOT_DIR/.config/kubernetes/vault
VAULT_TMP_DATA_DIR=$BACKEND_ROOT_DIR/.data/tmp/vault
VAULT_OTHER_DATA_DIR=$BACKEND_ROOT_DIR/.data/other/vault
UNSEAL_TOKEN_TRANSIT_FILE=$VAULT_OTHER_DATA_DIR/transit-unseal.json
AUTO_UNSEAL_TOKEN_FILE=$VAULT_TMP_DATA_DIR/autounseal-token.txt

# Source shared functions
source "$BACKEND_ROOT_DIR/scripts/functions/logs.sh"
source "$BACKEND_ROOT_DIR/scripts/functions/env.sh"

#
# Load environment variables from env file
#
source_env_file "$VAULT_OTHER_DATA_DIR/vault.env"
export VAULT_ADDR="$TRANSIT_VAULT_ADDR"

#
# Ensure Vault is running
#

# Wait until the Transit-Vault pod is created
POD_NAME=""
while [[ -z "$POD_NAME" ]]; do
  POD_NAME=$(kubectl get pods -n vault \
    -l app.kubernetes.io/instance=transit-vault,component=server \
    -o jsonpath="{.items[0].metadata.name}" 2>/dev/null || true)
  if [[ -z "$POD_NAME" ]]; then
    log_wait "Waiting for Transit-Vault pod to be created..."
    sleep 1
  fi
done

# Wait until the Transit-Vault pod is in Running phase
POD_PHASE=""
while [[ "$POD_PHASE" != "Running" ]]; do
  POD_PHASE=$(kubectl get pod "$POD_NAME" -n vault -o jsonpath="{.status.phase}" 2>/dev/null)
  if [[ "$POD_PHASE" != "Running" ]]; then
    log_wait "Waiting for $POD_NAME pod to be Running (current: $POD_PHASE)..."
    sleep 3
  fi
done

log "Vault pod $POD_NAME is Running."

#
# Transit Vault Installation and Initial Configuration
#

# 1. Initialize Vault
vault operator init -key-shares=1 -key-threshold=1 -format=json > "$UNSEAL_TOKEN_TRANSIT_FILE"

# 2. Unseal Vault
UNSEAL_KEY=$(jq -r '.unseal_keys_b64[0]' "$UNSEAL_TOKEN_TRANSIT_FILE")
vault operator unseal "$UNSEAL_KEY"

# 3. Login as root
ROOT_TOKEN=$(jq -r '.root_token' "$UNSEAL_TOKEN_TRANSIT_FILE")
export VAULT_TOKEN="$ROOT_TOKEN"

# 4. Enable Transit engine
vault secrets enable transit

# 5. Create key for Auto-Unseal
vault write -f transit/keys/autounseal

# 6. Write policy
vault policy write autounseal - <<EOF
path "transit/encrypt/autounseal" {
  capabilities = ["update"]
}
path "transit/decrypt/autounseal" {
  capabilities = ["update"]
}
EOF

# 7. Create and save auto-unseal token (direkt ohne wrapping)
AUTO_UNSEAL_TOKEN=$(vault token create -orphan -policy="autounseal" -period=24h -field=token)
if [[ -z "$AUTO_UNSEAL_TOKEN" ]]; then
  log_error "Failed to create auto-unseal token."
  exit 1
fi

# Speichere direkt in .txt Datei (nicht als env)
echo "$AUTO_UNSEAL_TOKEN" > "$VAULT_TMP_DATA_DIR/autounseal-token.txt"

#
# Clean up exported variables
#
unset VAULT_ADDR
unset VAULT_TOKEN

log "Transit Vault setup complete."