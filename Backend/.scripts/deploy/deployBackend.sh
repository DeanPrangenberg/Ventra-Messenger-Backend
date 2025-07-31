#!/bin/bash

set -euo pipefail

# === Configuration & Helper Functions ===
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BACKEND_ROOT_DIR="$SCRIPT_DIR/../.."
echo "[DEBUG] Script directory is: $SCRIPT_DIR"
echo "[DEBUG] Backend root directory is: $BACKEND_ROOT_DIR"

source "$BACKEND_ROOT_DIR/.scripts/functions/logs.sh"
source "$BACKEND_ROOT_DIR/.scripts/functions/env.sh"



# Function to wait for a Kubernetes job to complete
wait_for_job_completion() {
    local job_name="$1"
    local namespace="$2"
    local timeout="${3:-600}"
    
    log "Waiting for job '$job_name' in namespace '$namespace' to complete (timeout: ${timeout}s)..."
    local elapsed=0
    until kubectl get job "$job_name" -n "$namespace" -o jsonpath='{.status.succeeded}' 2>/dev/null | grep -q "1" || [ $elapsed -ge $timeout ]; do
        if kubectl get job "$job_name" -n "$namespace" -o jsonpath='{.status.failed}' 2>/dev/null | grep -q "[1-9]"; then
            error "Job '$job_name' failed."
            log "Fetching logs for failed job..."
            kubectl logs job/"$job_name" -n "$namespace" --all-containers=true || true
            return 1
        fi
        log "Waiting for job '$job_name' completion... (${elapsed}s elapsed)"
        sleep 10
        elapsed=$((elapsed + 10))
    done

    if [ $elapsed -ge $timeout ]; then
        error "Job '$job_name' did not complete within $timeout seconds."
        log "Describing job for more details..."
        kubectl describe job "$job_name" -n "$namespace"
        log "Fetching logs for timed out job..."
        kubectl logs job/"$job_name" -n "$namespace" --all-containers=true || true
        return 1
    fi

    log "Job '$job_name' completed successfully."
    return 0
}

# === Prerequisites Check ===
log "Checking prerequisites..."

# Check required scripts
REQUIRED_SCRIPTS=(
    "$SCRIPT_DIR/deploy-helm-charts.sh"
    "$SCRIPT_DIR/service-configurator/cert-manager.sh"
    "$SCRIPT_DIR/service-configurator/vault.sh"
)

for script in "${REQUIRED_SCRIPTS[@]}"; do
    if [[ ! -f "$script" ]]; then
        error "$script not found!"
        exit 1
    fi
done

# Check required Kubernetes files
K8S_FILES_DIR="$SCRIPT_DIR/../../.kubeConfig"
VAULT_INIT_RBAC_FILE="$K8S_FILES_DIR/vm-services/inti-vault/vault-init-rbac.yaml"
VAULT_INIT_JOB_FILE="$K8S_FILES_DIR/vm-services/inti-vault/vault-init-job.yaml"

if [[ ! -f "$VAULT_INIT_RBAC_FILE" ]] || [[ ! -f "$VAULT_INIT_JOB_FILE" ]]; then
    error "Required Kubernetes files not found."
    log "RBAC file: $VAULT_INIT_RBAC_FILE - $([[ -f "$VAULT_INIT_RBAC_FILE" ]] && echo "Found" || echo "Not Found")"
    log "Job file: $VAULT_INIT_JOB_FILE - $([[ -f "$VAULT_INIT_JOB_FILE" ]] && echo "Found" || echo "Not Found")"
    exit 1
fi

# Ensure CLI tools are installed
if [[ -f "$SCRIPT_DIR/getCLITools.sh" ]]; then
    chmod +x "$SCRIPT_DIR/getCLITools.sh"
    log "Checking/Installing CLI tools..."
    "$SCRIPT_DIR/getCLITools.sh"
else
    error "getCLITools.sh not found in $SCRIPT_DIR!"
    exit 1
fi

# === Deployment Steps ===

# Step 0: Build and Push Docker Images
echo "=============================================="
echo "  Step 0: Building and Pushing Docker Images"
echo "=============================================="
log "Starting Docker image build and push process..."
chmod +x "$SCRIPT_DIR/uploadImages.sh"
if ! (cd "$SCRIPT_DIR" && ./uploadImages.sh); then
    error "Docker image build and push failed!"
    exit 1
fi
log "Docker image build and push completed."

# Step 1.5: Create GHCR Secrets
echo "=============================================="
echo "  Step 0.5: Create GHCR Secrets"
echo "=============================================="

# Create GHCR Image Pull Secret in multiple namespaces
TARGET_NAMESPACES=("vault")

for TARGET_NAMESPACE in "${TARGET_NAMESPACES[@]}"; do
    log "Ensuring namespace '$TARGET_NAMESPACE' exists..."
    kubectl get namespace "$TARGET_NAMESPACE" >/dev/null 2>&1 || kubectl create namespace "$TARGET_NAMESPACE"
    log "Creating GHCR image pull secret in namespace '$TARGET_NAMESPACE'..."
    chmod +x "$SCRIPT_DIR/createGHCR-Secrets.sh"
    if ! (cd "$SCRIPT_DIR" && ./createGHCR-Secrets.sh "$TARGET_NAMESPACE"); then
        error "Failed to create GHCR image pull secret in namespace $TARGET_NAMESPACE!"
        exit 1
    fi
    log "GHCR image pull secret created in namespace '$TARGET_NAMESPACE'."
done

# Step 1: Deploy Helm Charts
echo "=============================================="
echo "  Step 1: Deploying Helm Charts"
echo "=============================================="
log "Starting Helm chart deployment..."
chmod +x "$SCRIPT_DIR/deploy-helm-charts.sh"
if ! (cd "$SCRIPT_DIR" && ./deploy-helm-charts.sh); then
    error "Helm chart deployment failed!"
    exit 1
fi

if ! source_env_file "$SCRIPT_DIR/tmp/.env"; then
    error ".env file not found after Helm chart deployment!"
    exit 1
fi
log "Helm chart deployment completed successfully."



# Step 2: Wait for Vault to be initialized and unsealed by Init-Container
echo "=============================================="
echo "  Step 2: Waiting for Vault Init/Unseal"
echo "=============================================="

log "Vault will be initialized and unsealed by the init-container."
log "Waiting for Vault pod to be ready..."
kubectl wait --for=condition=ready pod -l app.kubernetes.io/name=vault -n vault --timeout=300s

log "Waiting for Vault init/unseal secrets to be created..."
SECRETS_TIMEOUT=120
SECRETS_ELAPSED=0

until (kubectl get secret vault-init-secret -n vault >/dev/null 2>&1 && kubectl get secret vault-unseal-secret -n vault >/dev/null 2>&1) || [ $SECRETS_ELAPSED -ge $SECRETS_TIMEOUT ]; do
    log "Waiting for Vault secrets (init & unseal) to be created... (${SECRETS_ELAPSED}s elapsed)"
    sleep 5
    SECRETS_ELAPSED=$((SECRETS_ELAPSED + 5))
done

if [ $SECRETS_ELAPSED -ge $SECRETS_TIMEOUT ]; then
    error "Vault init/unseal secrets were not created within $SECRETS_TIMEOUT seconds."
    log "Listing secrets in vault namespace for debugging..."
    kubectl get secrets -n vault || true
    log "Checking Vault pod logs..."
    kubectl logs -l app.kubernetes.io/name=vault -n vault -c vault-init-unseal --tail=50 || true
    exit 1
fi

log "Vault init/unseal secrets found."

# Retrieve Vault Root Token
log "Retrieving root token from Kubernetes secret..."
VAULT_ROOT_TOKEN_FROM_SECRET=$(kubectl get secret vault-init-secret -n vault -o jsonpath='{.data.root-token}' | base64 -d)

if [[ -n "${VAULT_ROOT_TOKEN_FROM_SECRET}" ]]; then
    export VAULT_TOKEN="${VAULT_ROOT_TOKEN_FROM_SECRET}"
    log "VAULT_TOKEN successfully updated from Kubernetes secret (first 10 chars: ${VAULT_TOKEN:0:10}...)."
else
    error "Failed to retrieve root token from Kubernetes secret 'vault-init-secret'."
    log "Inspecting secret contents..."
    kubectl get secret vault-init-secret -n vault -o yaml || true
    exit 1
fi

# Step 3: Set up Vault PKI
echo "=============================================="
echo "  Step 3: Setting up Vault PKI"
echo "=============================================="
export VAULT_ADDR="${VAULT_ADDR:-http://127.0.0.1:8200}"
log "Using VAULT_ADDR: $VAULT_ADDR for Vault PKI setup."

chmod +x "$SCRIPT_DIR/service-configurator/vault.sh"
log "Starting Vault PKI setup..."
if ! (cd "$SCRIPT_DIR" && VAULT_TOKEN="$VAULT_TOKEN" ./service-configurator/vault.sh); then
    error "Vault PKI setup failed!"
    exit 1
fi
log "Vault PKI setup completed successfully."

# Step 4: Configure cert-manager
echo "=============================================="
echo "  Step 4: Configuring cert-manager"
echo "=============================================="
log "Reloading environment variables after Vault PKI setup..."
if ! source_env_file "$SCRIPT_DIR/tmp/.env"; then
    log_warn "Could not reload .env after Vault PKI setup."
fi

chmod +x "$SCRIPT_DIR/service-configurator/cert-manager.sh"
log "Starting cert-manager configuration..."
if ! (cd "$SCRIPT_DIR" && VAULT_TOKEN="$VAULT_TOKEN" ./service-configurator/cert-manager.sh); then
    error "cert-manager configuration failed!"
    exit 1
fi

# === Final Summary ===
log "Preparing final summary..."
if ! source_env_file "$SCRIPT_DIR/tmp/.env"; then
    log_warn "Could not reload final .env file."
fi

echo "=============================================="
echo " 🎉 Deployment completed successfully! 🎉"
echo "=============================================="