#!/bin/sh
set -euo pipefail

log() { echo "[vault-init] $*"; }
fail() { echo "[vault-init][ERROR] $*" >&2; exit 1; }

command -v vault >/dev/null 2>&1 || fail "vault CLI not found"
command -v jq >/dev/null 2>&1 || fail "jq not found"
command -v curl >/dev/null 2>&1 || fail "curl not found"
command -v shred >/dev/null 2>&1 || fail "shred not found"

export VAULT_ADDR="https://vm-vault:8200"
export VAULT_SKIP_VERIFY="true"

SECRETS_DIR="/vaultSecrets"
INIT_FILE="$SECRETS_DIR/vault-init.json"
CLIENT_CERTS_DIR="/vaultInitCerts"
CERT_AUTH_PATH="cert"
POLICY="service-cert-policy"

K8S_HOST="https://kubernetes.default.svc:443"
K8S_CA_CERT="/var/run/secrets/kubernetes.io/serviceaccount/ca.crt"
K8S_JWT="/var/run/secrets/kubernetes.io/serviceaccount/token"

mkdir -p "$SECRETS_DIR"

if vault status -format=json | jq -e '.initialized == true' >/dev/null 2>&1; then
  log "Vault is already initialized. Exiting."
  exit 0
fi
if [ -f "$INIT_FILE" ]; then
  log "Init file already exists. Exiting to avoid re-initialization."
  exit 0
fi

log "Starting Vault initialization"

vault operator init -key-shares=1 -key-threshold=1 -format=json > "$INIT_FILE" || fail "Vault initialization failed"
log "Vault initialized: Unseal key and root token saved"

UNSEAL_KEY=$(jq -r '.unseal_keys_b64[0]' "$INIT_FILE") || fail "Failed to extract unseal key"
ROOT_TOKEN=$(jq -r '.root_token' "$INIT_FILE") || fail "Failed to extract root token"

for i in $(seq 1 30); do
  if curl -sk https://vm-vault:8200/v1/sys/health | grep -q '"initialized":true'; then
    break
  fi
  log "Waiting for Vault to be ready... ($i/30)"
  sleep 2
done

vault operator unseal "$UNSEAL_KEY" || fail "Unseal failed"
export VAULT_TOKEN="$ROOT_TOKEN"
sleep 2

# Enable Kubernetes auth
if ! vault auth list | grep -q '^kubernetes/'; then
  vault auth enable kubernetes || fail "Failed to enable Kubernetes auth"
  log "Vault Kubernetes auth method enabled"
fi

# Configure Kubernetes auth (assumes Vault runs in cluster and can access K8s API)
vault write auth/kubernetes/config \
  token_reviewer_jwt=@$K8S_JWT \
  kubernetes_host="$K8S_HOST" \
  kubernetes_ca_cert=@"$K8S_CA_CERT" || fail "Failed to configure Kubernetes auth"
log "Vault Kubernetes auth configured"

# Enable PKI if not already
if ! vault read -format=json pki-int/config/urls >/dev/null 2>&1; then
  log "Configuring PKI..."
  vault secrets enable -path=pki-int pki || fail "Enable PKI failed"
  vault write pki-int/root/generate/internal \
    common_name="ca.services.ventra.internal" \
    ttl="87600h" \
    key_type="rsa" \
    key_bits=4096 || fail "Root CA generation failed"
fi

# Example: Setup for one service (repeat for each service as needed)
SERVICE="vm-api"
NAMESPACE="default"
SERVICE_ACCOUNT="${SERVICE}-sa"
VAULT_POLICY="${SERVICE}-policy"
VAULT_ROLE="${SERVICE}"

# Write policy for the service
cat > policy.hcl <<EOF
path "pki-int/issue/${SERVICE}" {
  capabilities = ["update"]
}
EOF
vault policy write "$VAULT_POLICY" policy.hcl || fail "Policy write failed"
rm -f policy.hcl

# Create Vault Kubernetes role for the service
vault write auth/kubernetes/role/"$VAULT_ROLE" \
  bound_service_account_names="$SERVICE_ACCOUNT" \
  bound_service_account_namespaces="$NAMESPACE" \
  policies="$VAULT_POLICY" \
  ttl=1h || fail "Failed to create Vault K8s role"

# Create PKI role for the service
vault write pki-int/roles/"$SERVICE" \
  allowed_domains="${SERVICE}.svc.cluster.local" \
  allow_subdomains=false \
  max_ttl="24h" || fail "Failed to create PKI role"

log "Vault setup for Kubernetes mTLS for $SERVICE completed."

log "Vault initialization and configuration completed successfully."
log "DEV TOKEN: $ROOT_TOKEN"