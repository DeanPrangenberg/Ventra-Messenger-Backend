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

mkdir -p "$SECRETS_DIR"

# Exit if already initialized (by Vault or file)
if vault status -format=json | jq -e '.initialized == true' >/dev/null 2>&1; then
  log "Vault is already initialized. Exiting."
  exit 0
fi
if [ -f "$INIT_FILE" ]; then
  log "Init file already exists. Exiting to avoid re-initialization."
  exit 0
fi

log "Starting Vault initialization"

# Initialize Vault
if ! vault operator init -key-shares=1 -key-threshold=1 -format=json > "$INIT_FILE"; then
  fail "Vault initialization failed"
fi
log "Vault initialized: Unseal key and root token saved"

UNSEAL_KEY=$(jq -r '.unseal_keys_b64[0]' "$INIT_FILE") || fail "Failed to extract unseal key"
ROOT_TOKEN=$(jq -r '.root_token' "$INIT_FILE") || fail "Failed to extract root token"

# Wait for Vault to be ready
for i in $(seq 1 30); do
  if curl -sk https://vm-vault:8200/v1/sys/health | grep -q '"initialized":true'; then
    break
  fi
  log "Waiting for Vault to be ready... ($i/30)"
  sleep 2
done

# Unseal Vault
if ! vault operator unseal "$UNSEAL_KEY"; then
  fail "Unseal failed"
fi

export VAULT_TOKEN="$ROOT_TOKEN"
sleep 2

# Enable cert auth if not already enabled
if ! vault auth list | grep -q '^cert/'; then
  if ! vault auth enable cert; then
    fail "Failed to enable cert auth"
  fi
  log "Vault cert auth method enabled"
fi

# Configure PKI and policy only if not already present
if ! vault read -format=json pki-int/config/urls >/dev/null 2>&1; then
  log "Configuring Vault for Zero Trust operation..."
  vault secrets enable -path=pki-int pki || fail "Enable PKI failed"
  vault write pki-int/root/generate/internal \
    common_name="ca.services.ventra.internal" \
    ttl="87600h" \
    key_type="rsa" \
    key_bits=4096 || fail "Root CA generation failed"
  vault write pki-int/roles/service-mtls \
    allowed_domains="services.ventra.internal" \
    allow_subdomains=true \
    max_ttl="24h" \
    generate_lease=true \
    enforce_hostnames=false || fail "Role creation failed"
cat > policy.hcl << EOF
path "pki-int/sign/service-mtls" {
  capabilities = ["update"]
}
path "pki-int/issue/service-mtls" {
  capabilities = ["read", "update"]
}
path "pki-int/ca/pem" {
  capabilities = ["read"]
}
EOF
  vault policy write service-cert-policy policy.hcl || fail "Policy write failed"
  rm -f policy.hcl
  log "Vault configuration completed: PKI, role, and policy are ready for mTLS"
else
  log "PKI engine is already configured, skipping setup"
fi

# Register client certs for cert auth
if [ -d "$CLIENT_CERTS_DIR" ]; then
  find "$CLIENT_CERTS_DIR" -type f -name "*.crt" | while read -r cert; do
    [ -e "$cert" ] || continue
    SERVICE_NAME=$(basename "$(dirname "$cert")")
    log "Registering client cert for $SERVICE_NAME"

    # Calculate SHA256 fingerprint of the cert
    CERT_FINGERPRINT=$(openssl x509 -in "$cert" -noout -fingerprint -sha256 | cut -d'=' -f2 | tr -d ':')

    vault delete auth/$CERT_AUTH_PATH/certs/$SERVICE_NAME >/dev/null 2>&1 || true

    if vault write auth/$CERT_AUTH_PATH/certs/$SERVICE_NAME \
      display_name="$SERVICE_NAME" \
      policies="$POLICY" \
      certificate=@"$cert"; then
      log "Registered $SERVICE_NAME with policy $POLICY (cert SHA256: $CERT_FINGERPRINT)"
    else
      log "Failed to register $SERVICE_NAME"
    fi
  done
else
  log "No client certs directory found at $CLIENT_CERTS_DIR, skipping client registration"
fi

# Revoke root token and securely delete sensitive files
# vault token revoke "$ROOT_TOKEN" || log "Root token already revoked or revoke failed"
# shred -u "$INIT_FILE" || rm -f "$INIT_FILE"
# log "Root token revoked and init file securely deleted"
# log "Vault is production-ready and initialized."

log "Vault initialization and configuration completed successfully."
log "DEV TOKEN: $ROOT_TOKEN"