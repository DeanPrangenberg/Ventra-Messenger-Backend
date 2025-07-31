#!/bin/bash
set -euo pipefail

CONFIG_FILE="/vault/config/vault-transit-config.hcl"


#
# Setting up paths and loading functions
#

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BACKEND_ROOT_DIR="$SCRIPT_DIR/../../.."
VAULT_CONFIG_DIR="$BACKEND_ROOT_DIR/.config/kubernetes/vault"
UNSEAL_FILE="$VAULT_CONFIG_DIR/transit-unseal.json"
TOKEN_FILE="$VAULT_CONFIG_DIR/autounseal-token.json"

HOST_IP=$(hostname -I | awk '{print $1}')

source "$BACKEND_ROOT_DIR/.scripts/functions/logs.sh"
source "$BACKEND_ROOT_DIR/.scripts/functions/env.sh"

#
# Load environment variables from env file
#

source_env_file "$VAULT_CONFIG_DIR/vault.env"

# 1. Initialize Vault
vault -address="$TRANSIT_VAULT_ADDR" operator init -key-shares=1 -key-threshold=1 -format=json > "$UNSEAL_FILE"

# 2. Unseal Vault
UNSEAL_KEY=$(jq -r '.unseal_keys_b64[0]' "$UNSEAL_FILE")
vault -address="$TRANSIT_VAULT_ADDR" operator unseal "$UNSEAL_KEY"

# 3. Login as root
ROOT_TOKEN=$(jq -r '.root_token' "$UNSEAL_FILE")
export VAULT_TOKEN="$ROOT_TOKEN"

# 4. Enable Transit engine
vault -address="$TRANSIT_VAULT_ADDR" secrets enable transit

# 5. Create key for Auto-Unseal
vault -address="$TRANSIT_VAULT_ADDR" write -f transit/keys/autounseal

# 6. Write policy
vault -address="$TRANSIT_VAULT_ADDR" policy write autounseal - <<EOF
path "transit/encrypt/autounseal" {
  capabilities = ["update"]
}
path "transit/decrypt/autounseal" {
  capabilities = ["update"]
}
EOF

# 7. Create token for Auto-Unseal
vault -address="$TRANSIT_VAULT_ADDR" token create -policy=autounseal -orphan -ttl=24h -format=json > "$TOKEN_FILE"

echo "Transit Vault setup complete."