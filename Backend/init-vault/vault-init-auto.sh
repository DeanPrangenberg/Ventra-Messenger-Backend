#!/bin/bash
# vault-init-auto.sh - Automatically initialize and unseal Vault, storing keys in Kubernetes secrets.

set -euo pipefail

echo "[INFO] Starting Vault auto-initialization and unseal process..."

# --- Configuration ---
VAULT_SERVICE_NAME="vault" # Name of the Vault service
VAULT_NAMESPACE="${VAULT_NAMESPACE:-vault}" # Namespace where Vault is deployed
VAULT_PORT="8200"
INIT_SECRET_NAME="vault-init-secret" # Name for the Kubernetes secret storing init keys
UNSEAL_SECRET_NAME="vault-unseal-secret" # Name for the Kubernetes secret storing the unseal key

# Construct the internal Vault address
VAULT_INTERNAL_ADDR="http://${VAULT_SERVICE_NAME}.${VAULT_NAMESPACE}.svc.cluster.local:${VAULT_PORT}"
export VAULT_ADDR="${VAULT_INTERNAL_ADDR}"
echo "[INFO] Targeting Vault at internal address: ${VAULT_ADDR}"

# --- Functions ---

# Wait for the Vault service to be resolvable
wait_for_service() {
    local service="$1"
    local retries=30
    local count=0
    echo "[INFO] Waiting for service ${service} to be resolvable..."
    until nslookup "${service}" >/dev/null 2>&1 || [ $count -eq $retries ]; do
        count=$((count + 1))
        echo "[INFO] Service ${service} not ready (attempt ${count}/${retries})..."
        sleep 5
    done

    if [ $count -eq $retries ]; then
        echo "[ERROR] Service ${service} not resolvable within timeout."
        exit 1
    fi
    echo "[INFO] Service ${service} is resolvable."
}

# Wait for the Vault API to respond (even if sealed)
wait_for_vault_api() {
    local retries=60
    local count=0
    echo "[INFO] Waiting for Vault API to respond at ${VAULT_ADDR}..."
    # Use a path that doesn't require auth, like sys/health, but ignore HTTP errors for now
    until curl -s -f -o /dev/null "${VAULT_ADDR}/v1/sys/health" || [ $count -eq $retries ]; do
        count=$((count + 1))
        echo "[INFO] Vault API not responding (attempt ${count}/${retries})..."
        sleep 5
    done

    if [ $count -eq $retries ]; then
        echo "[ERROR] Vault API did not respond within timeout."
        exit 1
    fi
    echo "[INFO] Vault API is responding."
}

# Check if Vault is already initialized
is_initialized() {
    if vault status -format=json 2>/dev/null | jq -e '.initialized == true' > /dev/null; then
        return 0 # true
    else
        return 1 # false
    fi
}

# Initialize Vault and store keys in a Kubernetes secret
init_vault() {
    echo "[INFO] Initializing Vault..."
    local init_output_json
    # Initialize with 1 key share for simplicity (NOT FOR PRODUCTION - use 5/3 or similar)
    init_output_json=$(vault operator init -key-shares=1 -key-threshold=1 -format=json)

    # Extract keys and token
    local unseal_key_b64
    local root_token
    unseal_key_b64=$(echo "${init_output_json}" | jq -r '.unseal_keys_b64[0]')
    root_token=$(echo "${init_output_json}" | jq -r '.root_token')

    if [[ -z "${unseal_key_b64}" || -z "${root_token}" ]]; then
        echo "[ERROR] Failed to extract unseal key or root token from init output."
        exit 1
    fi

    echo "[INFO] Vault initialized successfully."

    # Create Kubernetes secret for root token
    echo "[INFO] Creating Kubernetes secret '${INIT_SECRET_NAME}' for root token..."
    kubectl create secret generic "${INIT_SECRET_NAME}" \
        --namespace="${VAULT_NAMESPACE}" \
        --from-literal=root-token="${root_token}" \
        --from-literal=root-token-b64="$(echo -n "${root_token}" | base64)" \
        --from-literal=init-json="${init_output_json}" \
        --from-literal=init-json-b64="$(echo -n "${init_output_json}" | base64)" \
        --dry-run=client -o yaml | kubectl apply -f -

    # Create Kubernetes secret for unseal key (separate for potential future use, e.g., auto-unseal on restart)
    echo "[INFO] Creating Kubernetes secret '${UNSEAL_SECRET_NAME}' for unseal key..."
    kubectl create secret generic "${UNSEAL_SECRET_NAME}" \
        --namespace="${VAULT_NAMESPACE}" \
        --from-literal=unseal-key="${unseal_key_b64}" \
        --dry-run=client -o yaml | kubectl apply -f -

    echo "[SUCCESS] Vault keys stored in Kubernetes secrets."
    echo "[INFO] Root Token: ${root_token}"
    echo "[INFO] Unseal Key (Base64): ${unseal_key_b64}"

    # Export for potential later use in this script (though we'll read from secret)
    export VAULT_ROOT_TOKEN="${root_token}"
    export VAULT_UNSEAL_KEY_B64="${unseal_key_b64}"
}

# Check if Vault is already unsealed
is_sealed() {
     if vault status -format=json 2>/dev/null | jq -e '.sealed == true' > /dev/null; then
        return 0 # true, it is sealed
    else
        return 1 # false, it is unsealed
    fi
}

# Unseal Vault using the key from the Kubernetes secret
unseal_vault() {
    echo "[INFO] Retrieving unseal key from Kubernetes secret '${UNSEAL_SECRET_NAME}'..."
    local unseal_key_b64_from_secret
    unseal_key_b64_from_secret=$(kubectl get secret "${UNSEAL_SECRET_NAME}" --namespace="${VAULT_NAMESPACE}" -o jsonpath='{.data.unseal-key}' 2>/dev/null)

    if [[ -z "${unseal_key_b64_from_secret}" ]]; then
        echo "[ERROR] Failed to retrieve unseal key from secret '${UNSEAL_SECRET_NAME}'."
        exit 1
    fi

    # Decode the base64 key
    local unseal_key_decoded
    unseal_key_decoded=$(echo "${unseal_key_b64_from_secret}" | base64 -d)

    echo "[INFO] Unsealing Vault..."
    vault operator unseal "${unseal_key_decoded}"
    echo "[SUCCESS] Vault unsealed."
}

# --- Main Execution ---

# Wait for Vault service and API
wait_for_service "${VAULT_SERVICE_NAME}.${VAULT_NAMESPACE}.svc.cluster.local"
wait_for_vault_api

# Check initialization status
if is_initialized; then
    echo "[INFO] Vault is already initialized."
else
    echo "[INFO] Vault is not initialized. Initializing..."
    init_vault
fi

# Check seal status
if is_sealed; then
    echo "[INFO] Vault is sealed. Unsealing..."
    unseal_vault
else
    echo "[INFO] Vault is already unsealed."
fi

# Final status check
echo "[INFO] Final Vault status check:"
vault status || true # Don't fail the job if status command has issues

echo "[INFO] Vault auto-initialization and unseal process completed successfully."
exit 0