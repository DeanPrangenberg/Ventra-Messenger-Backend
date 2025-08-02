#!/bin/bash
set -euo pipefail

# Script header
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BACKEND_ROOT_DIR="$SCRIPT_DIR/../../../.."
VAULT_CONFIG_DIR=$BACKEND_ROOT_DIR/.config/kubernetes/vault
VAULT_TMP_DATA_DIR=$BACKEND_ROOT_DIR/.data/tmp/vault
VAULT_OTHER_DATA_DIR=$BACKEND_ROOT_DIR/.data/other/vault
AUTO_UNSEAL_TOKEN_FILE=$VAULT_TMP_DATA_DIR/autounseal-token.txt
CA_CERT_FILE=$VAULT_OTHER_DATA_DIR/ca-cert.pem

# Load and set the auto-unseal token FIRST
if [ ! -f "$AUTO_UNSEAL_TOKEN_FILE" ]; then
    error "Auto-unseal token file $AUTO_UNSEAL_TOKEN_FILE does not exist."
    exit 1
fi

AUTO_UNSEAL_TOKEN=$(tr -d '"\n' < "$AUTO_UNSEAL_TOKEN_FILE")
export VAULT_TOKEN="$AUTO_UNSEAL_TOKEN"

source "$BACKEND_ROOT_DIR/.scripts/functions/logs.sh"
source "$BACKEND_ROOT_DIR/.scripts/functions/env.sh"

source_env_file "$VAULT_OTHER_DATA_DIR/vault.env"
export VAULT_ADDR="$PKI_VAULT_ADDR"

# Wait for PKI-Vault pod to be running (existing code)
POD_NAME=""
while [[ -z "$POD_NAME" ]]; do
  POD_NAME=$(kubectl get pods -n vault \
    -l app.kubernetes.io/instance=pki-vault,component=server \
    -o jsonpath="{.items[0].metadata.name}" 2>/dev/null || true)
  if [[ -z "$POD_NAME" ]]; then
    log_wait "Waiting for PKI-Vault pod to be created..."
    sleep 1
  fi
done

POD_PHASE=""
while [[ "$POD_PHASE" != "Running" ]]; do
  POD_PHASE=$(kubectl get pod "$POD_NAME" -n vault -o jsonpath="{.status.phase}" 2>/dev/null)
  if [[ "$POD_PHASE" != "Running" ]]; then
    log_wait "Waiting for $POD_NAME pod to be Running (current: $POD_PHASE)..."
    sleep 3
  fi
done

log "Vault pod $POD_NAME is Running."

# Check if Vault is initialized
if ! vault status &>/dev/null; then
    log "Initializing PKI-Vault with auto-unseal..."
    sleep 2

    # Initialize with recovery keys (not unseal keys) since we're using auto-unseal
    vault operator init > "$VAULT_OTHER_DATA_DIR/pki-vault-init.txt"
    log "PKI-Vault initialized with auto-unseal (recovery keys)."
else
    log "PKI-Vault already initialized."
fi

log "Waiting for PKI-Vault to auto-unseal..."
while true; do
    SEAL_STATUS=$(kubectl exec -n vault pki-vault-0 -- vault status -format=json 2>/dev/null | jq -r '.sealed' 2>/dev/null)

    if [[ "$SEAL_STATUS" == "false" ]]; then
        echo "PKI-Vault is unsealed."
        break
    else
        echo "Waiting for PKI-Vault to auto-unseal... (current status: sealed=$SEAL_STATUS)"
        sleep 3
    fi
done

# Get root token from initialization output for further operations
ROOT_TOKEN=$(grep 'Initial Root Token:' "$VAULT_OTHER_DATA_DIR/pki-vault-init.txt" | awk '{print $NF}')
export VAULT_TOKEN="$ROOT_TOKEN"

# Enable PKI secrets engine if not already enabled
if ! vault secrets list | grep -q '^pki/'; then
    log "Enabling PKI-Vault secrets engine..."
    vault secrets enable -path=pki pki
    log "PKI-Vault secrets engine enabled at path /pki."
else
    log "PKI secrets engine already enabled at /pki."
fi

# Generate root CA if not already present
if ! vault read -field=certificate pki/cert/ca &>/dev/null; then
    log "Generating root CA certificate..."
    vault write -field=certificate pki/root/generate/internal \
        common_name="ventra.cluster" ttl=87600h > "$CA_CERT_FILE"
    log "Root CA certificate generated and saved to $CA_CERT_FILE."
else
    log "Root CA certificate already exists."
fi

# Configure issuing and CRL URLs
vault write pki/config/urls \
    issuing_certificates="$PKI_VAULT_ADDR/v1/pki/ca" \
    crl_distribution_points="$PKI_VAULT_ADDR/v1/pki/crl"
log "PKI-Vault CA URLs configured."

#
# PKI Vault cert-manager Configuration
#

# Create or update PKI role for cert-manager
vault write pki/roles/cert-manager \
  allowed_domains="ventra.cluster" \
  allow_subdomains=true \
  max_ttl="48h" \
  allow_any_name=true \
  allow_bare_domains=true \
  key_type="rsa" \
  key_bits=4096 \
  key_usage="DigitalSignature,KeyEncipherment,KeyAgreement" \
  require_cn=false
log "PKI role for cert-manager created or updated."

# Enable Kubernetes auth method if not already enabled
if ! vault auth list | grep -q '^kubernetes/'; then
    log "Enabling Kubernetes auth method..."
    vault auth enable kubernetes
    log "Kubernetes auth method enabled."
else
    log "Kubernetes auth method already enabled."
fi

# Configure Kubernetes auth method
log "Configuring Kubernetes auth method..."
kubectl exec -n vault pki-vault-0 -- /bin/sh -c \
  "VAULT_TOKEN=$VAULT_TOKEN vault write auth/kubernetes/config \
    kubernetes_host='https://kubernetes.default.svc' \
    kubernetes_ca_cert=@/var/run/secrets/kubernetes.io/serviceaccount/ca.crt \
    token_reviewer_jwt=@/var/run/secrets/kubernetes.io/serviceaccount/token"

# Create Vault policy for cert-manager
vault policy write cert-manager <<EOF
path "pki/issue/cert-manager" {
  capabilities = ["create"]
}
EOF

# Kubernetes Auth Role: binds sa to policy
vault write auth/kubernetes/role/cert-manager \
  bound_service_account_names=vault-issuer \
  bound_service_account_namespaces=cert-manager \
  policies=cert-manager \
  ttl=24h


log "PKI Vault status:"
vault status

log "PKI Vault setup complete."