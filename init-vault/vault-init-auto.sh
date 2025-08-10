#!/bin/bash
# vault-init-auto.sh - Automatically initialize and unseal Vault, storing keys in Kubernetes secrets.
set -euo pipefail

# --- Configuration ---
VAULT_SERVICE_NAME="vault"
VAULT_NAMESPACE="${VAULT_NAMESPACE:-vault}"
VAULT_PORT="8200"
INIT_SECRET_NAME="vault-init-secret"
UNSEAL_SECRET_NAME="vault-unseal-secret"
CERT_MANAGER_TOKEN_SECRET="vault-cert-manager-token" # Added here for clarity

# Construct the internal Vault address
VAULT_INTERNAL_ADDR="http://${VAULT_SERVICE_NAME}.${VAULT_NAMESPACE}.svc.cluster.local:${VAULT_PORT}"
export VAULT_ADDR="${VAULT_INTERNAL_ADDR}"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Logging functions with timestamps
log() {
    echo -e "${GREEN}[$(date +'%Y-%m-%d %H:%M:%S')] [INFO]${NC} $1"
}

log_detail() {
    echo -e "${BLUE}[$(date +'%Y-%m-%d %H:%M:%S')] [DETAIL]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[$(date +'%Y-%m-%d %H:%M:%S')] [WARN]${NC} $1" >&2
}

error() {
    echo -e "${RED}[$(date +'%Y-%m-%d %H:%M:%S')] [ERROR]${NC} $1" >&2
    # Attempt cleanup of potentially partially created secrets on error
    kubectl delete secret "${INIT_SECRET_NAME}" --namespace="${VAULT_NAMESPACE}" --ignore-not-found >/dev/null 2>&1 || true
    kubectl delete secret "${UNSEAL_SECRET_NAME}" --namespace="${VAULT_NAMESPACE}" --ignore-not-found >/dev/null 2>&1 || true
    exit 1
}

log "Starting Vault auto-initialization and unseal process..."
log "Last script update: 23:26 2025-07-30"
log_detail "Targeting Vault at internal address: ${VAULT_ADDR}"
log_detail "Using Kubernetes namespace: ${VAULT_NAMESPACE}"

# --- Functions ---
# Wait for the Vault service to be resolvable
wait_for_service() {
    local service="$1"
    local retries=30
    local count=0
    log "Waiting for service ${service} to be resolvable..."
    until nslookup "${service}" >/dev/null 2>&1 || [ $count -eq $retries ]; do
        count=$((count + 1))
        log_detail "Service ${service} not ready (attempt ${count}/${retries})..."
        sleep 5
    done
    if [ $count -eq $retries ]; then
        error "Service ${service} not resolvable within timeout."
    fi
    log "Service ${service} is resolvable."
}

# Wait for the Vault API to respond (even if sealed)
wait_for_vault_api() {
    local retries=60
    local count=0
    local http_code
    log "Waiting for Vault API to respond at ${VAULT_ADDR}..."
    # Accept codes indicating Vault is alive, even if not ready
    until http_code=$(curl -s -o /dev/null -w "%{http_code}" "${VAULT_ADDR}/v1/sys/health" 2>/dev/null) && [[ "$http_code" =~ ^(200|429|501|503)$ ]] || [ $count -eq $retries ]; do
        ount=$((count + 1))
        log_detail "Vault API not responding (attempt ${count}/${retries}), HTTP code: ${http_code:-0}..."
        sleep 5
    done
    if [ $count -eq $retries ]; then
        error "Vault API did not respond within timeout. Last HTTP code: ${http_code:-0}."
    fi
    log "Vault API is responding (HTTP $http_code)."
}

# Check if Vault is already initialized
is_initialized() {
    local init_status
    log_detail "Checking Vault initialization status..."
    # Use curl for robustness, handle potential jq errors
    init_status=$(curl -s "${VAULT_ADDR}/v1/sys/health" 2>/dev/null | jq -r '.initialized' 2>/dev/null) || {
        log_warn "Could not determine initialization status via API, assuming not initialized."
        return 1 # Assume not initialized if check fails
    }
    if [[ "$init_status" == "true" ]]; then
        log_detail "Vault is already initialized."
        return 0 # true
    else
        log_detail "Vault is not initialized."
        return 1 # false
    fi
}

# Initialize Vault and store keys in Kubernetes secrets
init_vault() {
    log "Initializing Vault..."
    local init_output_json
    # --- PRODUCTION READINESS: Use stronger key shares/threshold ---
    # For production, consider 5/3 instead of 1/1
    # init_output_json=$(vault operator init -key-shares=5 -key-threshold=3 -format=json)
    # For now, keeping 1/1 for simplicity as requested, but noting it's not production standard.
    init_output_json=$(vault operator init -key-shares=1 -key-threshold=1 -format=json)

    # Extract keys and token
    local unseal_key_b64
    local root_token
    unseal_key_b64=$(echo "${init_output_json}" | jq -r '.unseal_keys_b64[0]')
    root_token=$(echo "${init_output_json}" | jq -r '.root_token')

    if [[ -z "${unseal_key_b64}" || -z "${root_token}" ]]; then
        error "Failed to extract unseal key or root token from init output."
    fi

    log "Vault initialized successfully."
    log_detail "Root Token ID (first 10 chars): ${root_token:0:10}..."
    log_detail "Unseal Key ID (first 10 chars): ${unseal_key_b64:0:10}..."

    # Create Kubernetes secret for root token
    log "Creating Kubernetes secret '${INIT_SECRET_NAME}' for root token..."
    if kubectl create secret generic "${INIT_SECRET_NAME}" \
        --namespace="${VAULT_NAMESPACE}" \
        --from-literal=root-token="${root_token}" \
        --from-literal=root-token-b64="$(echo -n "${root_token}" | base64)" \
        --from-literal=init-json="${init_output_json}" \
        --from-literal=init-json-b64="$(echo -n "${init_output_json}" | base64)" \
        --dry-run=client -o yaml | kubectl apply -f -; then
        log "Successfully created secret '${INIT_SECRET_NAME}'."
    else
        error "Failed to create Kubernetes secret '${INIT_SECRET_NAME}'."
    fi

    # Create Kubernetes secret for unseal key
    log "Creating Kubernetes secret '${UNSEAL_SECRET_NAME}' for unseal key..."
    if kubectl create secret generic "${UNSEAL_SECRET_NAME}" \
        --namespace="${VAULT_NAMESPACE}" \
        --from-literal=unseal-key="${unseal_key_b64}" \
        --dry-run=client -o yaml | kubectl apply -f -; then
        log "Successfully created secret '${UNSEAL_SECRET_NAME}'."
    else
        # Cleanup the first secret if the second fails
        kubectl delete secret "${INIT_SECRET_NAME}" --namespace="${VAULT_NAMESPACE}" --ignore-not-found >/dev/null 2>&1 || true
        error "Failed to create Kubernetes secret '${UNSEAL_SECRET_NAME}'."
    fi

    log "Vault keys stored in Kubernetes secrets."

    export VAULT_ROOT_TOKEN="${root_token}"
    export VAULT_UNSEAL_KEY_B64="${unseal_key_b64}"

    if is_sealed; then
        log "Vault is sealed. Proceeding with unseal process..."
        unseal_vault
    else
        log "Vault is already unsealed. Skipping unseal process."
    fi

    if is_initialized && ! is_sealed; then
        log "Vault is initialized and unsealed. Setting up cert-manager integration..."
        setup_cert_manager
    else
        log_warn "Vault is not in a state ready for cert-manager setup (initialized and unsealed). Skipping cert-manager setup."
    fi
}

# Setup cert-manager policy and token
setup_cert_manager() {
    local CERT_MANAGER_POLICY_NAME="vault-cert-manager-token"
    local CERT_MANAGER_POLICY_FILE="/tmp/cert-manager-admin-policy.hcl"

    log "Setting up cert-manager integration..."
    log_detail "Creating cert-manager policy: ${CERT_MANAGER_POLICY_NAME}"

    cat > "$CERT_MANAGER_POLICY_FILE" <<EOF
path "pki_int/sign/internal-services" {
  capabilities = ["update"]
}

path "pki_int/issue/internal-services" {
  capabilities = ["update"]
}

path "pki_int/cert/ca" {
  capabilities = ["read"]
}
EOF

    # Get root token if not already set
    local vault_token="${VAULT_ROOT_TOKEN:-$(kubectl get secret "${INIT_SECRET_NAME}" --namespace="${VAULT_NAMESPACE}" -o jsonpath='{.data.root-token}' | base64 -d)}"
    export VAULT_TOKEN="${vault_token}"

    log_detail "Writing policy to Vault..."
    if vault policy write "$CERT_MANAGER_POLICY_NAME" "$CERT_MANAGER_POLICY_FILE"; then
        log_detail "Policy '${CERT_MANAGER_POLICY_NAME}' created successfully."
    else
        error "Failed to create Vault policy '${CERT_MANAGER_POLICY_NAME}'."
    fi

    log "Creating dedicated cert-manager token..."
    local cert_manager_token
    cert_manager_token=$(vault token create -policy="$CERT_MANAGER_POLICY_NAME" -ttl=1h -format=json | jq -r .auth.client_token)

    if [[ -z "$cert_manager_token" ]]; then
        error "Failed to create cert-manager token."
    fi

    log "Storing cert-manager token in Kubernetes secret '${CERT_MANAGER_TOKEN_SECRET}'..."
    if kubectl create secret generic "${CERT_MANAGER_TOKEN_SECRET}" \
        --namespace="${VAULT_NAMESPACE}" \
        --from-literal=token="${cert_manager_token}" \
        --dry-run=client -o yaml | kubectl apply -f -; then
        log "Successfully created cert-manager token secret."
    else
        error "Failed to create cert-manager token secret."
    fi

    rm -f "$CERT_MANAGER_POLICY_FILE"
    log "cert-manager token and policy setup complete."
}

# Check if Vault is already unsealed
is_sealed() {
    local seal_status
    log_detail "Checking Vault seal status..."
    # Use curl for robustness
    seal_status=$(curl -s "${VAULT_ADDR}/v1/sys/health" 2>/dev/null | jq -r '.sealed' 2>/dev/null) || {
        log_warn "Could not determine seal status via API, assuming sealed."
        return 0 # Assume sealed if check fails
    }
    if [[ "$seal_status" == "true" ]]; then
        log_detail "Vault is currently sealed."
        return 0 # true, it is sealed
    else
        log_detail "Vault is currently unsealed."
        return 1 # false, it is unsealed
    fi
}

# Unseal Vault using the key from the Kubernetes secret
unseal_vault() {
    log "Retrieving unseal key from Kubernetes secret '${UNSEAL_SECRET_NAME}'..."
    local unseal_key_b64_from_secret
    unseal_key_b64_from_secret=$(kubectl get secret "${UNSEAL_SECRET_NAME}" --namespace="${VAULT_NAMESPACE}" -o jsonpath='{.data.unseal-key}' 2>/dev/null)
    if [[ -z "${unseal_key_b64_from_secret}" ]]; then
        error "Failed to retrieve unseal key from secret '${UNSEAL_SECRET_NAME}'."
    fi

    local unseal_key_decoded
    unseal_key_decoded=$(echo "${unseal_key_b64_from_secret}" | base64 -d)
    log "Unsealing Vault..."

    local unseal_response
    # Capture both stdout and stderr
    unseal_response=$(vault operator unseal "${unseal_key_decoded}" 2>&1) || {
        error "Failed to unseal Vault. Response: $unseal_response"
    }

    # Check response for success or definitive failure
    if echo "$unseal_response" | grep -q '"sealed": false'; then
        log "Vault successfully unsealed."
    elif echo "$unseal_response" | grep -q '"progress": 1'; then
        # For 1/1 unseal, progress 1 after unseal usually means success or a race condition.
        # Let's double-check the status via API after a short delay.
        log_detail "Waiting for seal status confirmation..."
        sleep 2
        if is_sealed; then
            error "Vault unseal command executed but Vault is still sealed. Response: $unseal_response"
        else
            log "Vault successfully unsealed."
        fi
    else
        log_warn "Unexpected unseal response (might still be OK): $unseal_response"
        # Final check via API
        log_detail "Performing final seal status check..."
        sleep 2
        if is_sealed; then
            error "Vault is still sealed after unseal command."
        else
            log "Vault is now unsealed."
        fi
    fi
}

# --- Main Execution ---
log "=== Vault Initialization and Unseal Process Started ==="
# Wait for Vault service and API
wait_for_service "${VAULT_SERVICE_NAME}.${VAULT_NAMESPACE}.svc.cluster.local"
wait_for_vault_api
# Check initialization status
if is_initialized; then
    log "Vault is already initialized. Skipping initialization."
else
    log "Vault is not initialized. Proceeding with initialization..."
    init_vault
fi
# Check seal status
if is_sealed; then
    log "Vault is sealed. Proceeding with unseal process..."
    unseal_vault
else
    log "Vault is already unsealed. Skipping unseal process."
fi

# Setup cert-manager integration AFTER Vault is unsealed
# Check if Vault is initialized (it should be by now) and unsealed
if is_initialized && ! is_sealed; then
    log "Vault is initialized and unsealed. Setting up cert-manager integration..."
    setup_cert_manager
else
    log_warn "Vault is not in a state ready for cert-manager setup (initialized and unsealed). Skipping cert-manager setup."
fi

# Final status check
log "Performing final Vault status check..."
# Use a timeout for the final check to prevent hanging
if timeout 30 vault status; then
    log "Final status check successful."
else
    log_warn "Final vault status command failed or timed out, but this might be transient. Vault should be initialized and unsealed."
fi

log "=== Vault auto-initialization and unseal process completed successfully ==="
exit 0