#!/bin/bash
# deployBackend.sh - Orchestrates the complete deployment of the Ventra stack with cert-manager integration

set -euo pipefail # Exit on error, undefined vars, pipe failures

# === Configuration & Helper Functions ===

# Determine the directory where this script resides
# This allows calling the script from any location
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
echo "[DEBUG] Script directory is: $SCRIPT_DIR"

# Logging functions
log() {
    echo "[INFO] $1"
}

log_warn() {
    echo "[WARN] $1" >&2
}

log_error() {
    echo "[ERROR] $1" >&2
}

# Ensure required CLI tools are installed
if [[ -f "$SCRIPT_DIR/getCLITools.sh" ]]; then
    chmod +x "$SCRIPT_DIR/getCLITools.sh"
    "$SCRIPT_DIR/getCLITools.sh"
else
    log_error "getCLITools.sh not found in $SCRIPT_DIR!"
    exit 1
fi

# Function to source environment files safely
source_env_file() {
    local env_file="$1"
    if [[ -f "$env_file" ]]; then
        log "Loading environment variables from $env_file"
        # Filter out comments and empty lines, then export
        while IFS= read -r line || [[ -n "$line" ]]; do
            # Skip empty lines and comments
            [[ -z "$line" || "$line" =~ ^[[:space:]]*# ]] && continue
            # Basic check for valid env var format
            if [[ "$line" =~ ^[a-zA-Z_][a-zA-Z0-9_]*= ]]; then
                export "$line"
            else
                log_warn "Skipping invalid line in $env_file: $line"
            fi
        done < <(grep -v '^#' "$env_file" | grep -v '^$')
    else
        log_error "$env_file not found!"
        return 1
    fi
}

# Function to wait for a Kubernetes job to complete
wait_for_job_completion() {
    local job_name="$1"
    local namespace="$2"
    local timeout="${3:-600}" # Default 10 minutes

    log "Waiting for job '$job_name' in namespace '$namespace' to complete (timeout: ${timeout}s)..."
    local elapsed=0
    until kubectl get job "$job_name" -n "$namespace" -o jsonpath='{.status.succeeded}' 2>/dev/null | grep -q "1" || [ $elapsed -ge $timeout ]; do
        # Check if it failed
        if kubectl get job "$job_name" -n "$namespace" -o jsonpath='{.status.failed}' 2>/dev/null | grep -q "[1-9]"; then
            log_error "Job '$job_name' failed."
            kubectl logs job/"$job_name" -n "$namespace" --all-containers=true || true
            return 1
        fi
        log "[INFO] Waiting for job '$job_name' completion... (${elapsed}s elapsed)"
        sleep 10
        elapsed=$((elapsed + 10))
    done

    if [ $elapsed -ge $timeout ]; then
        log_error "Job '$job_name' did not complete within $timeout seconds."
        kubectl describe job "$job_name" -n "$namespace"
        kubectl logs job/"$job_name" -n "$namespace" --all-containers=true || true
        return 1
    fi
    log "Job '$job_name' completed successfully."
    return 0
}

# === 1. Check prerequisites ===

echo "=============================================="
echo "  Ventra Stack Deployment Orchestrator"
echo "=============================================="

if [[ $# -ne 1 ]] && [[ -z "${VAULT_TOKEN:-}" ]]; then
    log_error "Vault root token is required."
    echo "       Either pass it as an argument: ./deployBackend.sh <vault-root-token>"
    echo "       Or set it as an environment variable: export VAULT_TOKEN='s.token' && ./deployBackend.sh"
    exit 1
fi

VAULT_ROOT_TOKEN="${1:-$VAULT_TOKEN}"

# Check if the main scripts are in the expected locations relative to this script
REQUIRED_SCRIPTS=(
    "$SCRIPT_DIR/deploy-helm-charts.sh"
    "$SCRIPT_DIR/service-configurator/cert-manager.sh"
    "$SCRIPT_DIR/service-configurator/vault.sh"
)

for script in "${REQUIRED_SCRIPTS[@]}"; do
    if [[ ! -f "$script" ]]; then
        log_error "$script not found!"
        exit 1
    fi
done

K8S_FILES_DIR="$SCRIPT_DIR/../../.kubeConfig"
VAULT_INIT_RBAC_FILE="$K8S_FILES_DIR/vm-services/inti-vault/vault-init-rbac.yaml"
VAULT_INIT_JOB_FILE="$K8S_FILES_DIR/vm-services/inti-vault/vault-init-job.yaml"

# Check if the required Kubernetes files exist
if [[ ! -f "$VAULT_INIT_RBAC_FILE" ]]; then
    log_error "Vault init RBAC file not found: $VAULT_INIT_RBAC_FILE"
    exit 1
fi
if [[ ! -f "$VAULT_INIT_JOB_FILE" ]]; then
    log_error "Vault init job file not found: $VAULT_INIT_JOB_FILE"
    exit 1
fi

# === 2. Deploy Helm charts (includes cert-manager installation) ===

echo ""
echo "----------------------------------------------"
echo " Step 1: Deploying Helm Charts (deploy-helm-charts.sh)"
echo "----------------------------------------------"

chmod +x "$SCRIPT_DIR/deploy-helm-charts.sh"
# Execute in the script's directory to ensure relative paths work
if ! (cd "$SCRIPT_DIR" && ./deploy-helm-charts.sh); then
    log_error "Helm chart deployment failed!"
    exit 1
fi

# Load environment variables from the first step
if ! source_env_file "$SCRIPT_DIR/tmp/.env"; then
    log_error ".env file not found after Helm chart deployment!"
    exit 1
fi

log "Helm chart deployment completed successfully."

# === 2.5. Initialize and Unseal Vault Automatically ===

echo ""
echo "----------------------------------------------"
echo " Step 1.5: Initializing and Unsealing Vault (Auto)"
echo "----------------------------------------------"

# Apply RBAC for the init job
log "Applying Vault init/unseal RBAC from $VAULT_INIT_RBAC_FILE..."
kubectl apply -f "$VAULT_INIT_RBAC_FILE"

# Apply the init job
log "Applying Vault init/unseal job from $VAULT_INIT_JOB_FILE..."
kubectl apply -f "$VAULT_INIT_JOB_FILE"

# Wait for the job to complete successfully
if ! wait_for_job_completion "vault-init-unseal-job" "vault" 600; then
    log_error "Vault auto-initialization/unseal job failed or timed out."
    exit 1
fi

log "Vault auto-initialization and unseal completed."

# === Update VAULT_TOKEN from Kubernetes Secret ===

log "Retrieving root token from Kubernetes secret for subsequent steps..."
VAULT_ROOT_TOKEN_FROM_SECRET=""
# Use a loop with a timeout to wait for the secret to be created by the job
SECRETS_TIMEOUT=120
SECRETS_ELAPSED=0
until kubectl get secret vault-init-secret -n vault >/dev/null 2>&1 || [ $SECRETS_ELAPSED -ge $SECRETS_TIMEOUT ]; do
    log "[INFO] Waiting for vault-init-secret to be created... (${SECRETS_ELAPSED}s elapsed)"
    sleep 5
    SECRETS_ELAPSED=$((SECRETS_ELAPSED + 5))
done

if [ $SECRETS_ELAPSED -ge $SECRETS_TIMEOUT ]; then
    log_error "vault-init-secret was not created within $SECRETS_TIMEOUT seconds."
    # List secrets in vault namespace for debugging
    kubectl get secrets -n vault || true
    exit 1
fi

# Now retrieve the token
VAULT_ROOT_TOKEN_FROM_SECRET=$(kubectl get secret vault-init-secret -n vault -o jsonpath='{.data.root-token}' | base64 -d)

if [[ -n "${VAULT_ROOT_TOKEN_FROM_SECRET}" ]]; then
    # Update the VAULT_TOKEN for the rest of this script
    export VAULT_TOKEN="${VAULT_ROOT_TOKEN_FROM_SECRET}"
    log "[INFO] VAULT_TOKEN successfully updated from Kubernetes secret (first 10 chars: ${VAULT_TOKEN:0:10}...)."
else
    log_error "Failed to retrieve root token from Kubernetes secret 'vault-init-secret'."
    # List keys in the secret for debugging
    kubectl get secret vault-init-secret -n vault -o jsonpath='{.data}' || true
    echo ""
    exit 1
fi

# === 3. Set up Vault PKI (with cert-manager integration) ===

echo ""
echo "----------------------------------------------"
echo " Step 2: Setting up Vault PKI (service-configurator/vault.sh)"
echo "----------------------------------------------"

# Ensure VAULT_ADDR is set correctly for the PKI script
# It should use the value from .env or default
export VAULT_ADDR="${VAULT_ADDR:-http://127.0.0.1:8200}"
log "[INFO] Using VAULT_ADDR: $VAULT_ADDR for Vault PKI setup."

chmod +x "$SCRIPT_DIR/service-configurator/vault.sh"
# Execute in the script's directory to ensure relative paths work
if ! (cd "$SCRIPT_DIR" && VAULT_TOKEN="$VAULT_TOKEN" ./service-configurator/vault.sh); then
    log_error "Vault PKI setup failed!"
    exit 1
fi

log "Vault PKI setup completed successfully."

# === 4. Configure cert-manager ===

echo ""
echo "----------------------------------------------"
echo " Step 3: Configuring cert-manager (service-configurator/cert-manager.sh)"
echo "----------------------------------------------"

# Reload .env one final time in case the PKI script updated it
if ! source_env_file "$SCRIPT_DIR/tmp/.env"; then
    log_warn "Could not reload .env after Vault PKI setup."
fi

chmod +x "$SCRIPT_DIR/service-configurator/cert-manager.sh"
# Execute in the script's directory to ensure relative paths work
# Pass the dynamically retrieved VAULT_TOKEN
if ! (cd "$SCRIPT_DIR" && VAULT_TOKEN="$VAULT_TOKEN" ./service-configurator/cert-manager.sh); then
    log_error "cert-manager configuration failed!"
    exit 1
fi

# Load environment variables from the cert-manager configuration
if ! source_env_file "$SCRIPT_DIR/tmp/.env.cm"; then
    log_warn ".env.cm file not found after cert-manager configuration. Continuing..."
fi

log "cert-manager configuration completed successfully."

# === 5. Final Summary ===

# Reload .env one final time in case any script updated it
if ! source_env_file "$SCRIPT_DIR/tmp/.env"; then
    log_warn "Could not reload final .env file."
fi

echo ""
echo "=============================================="
echo " 🎉 Deployment completed successfully! 🎉"
echo "=============================================="
echo ""
echo "Generated Files (in $SCRIPT_DIR/tmp/):"
echo "  - .env       : Environment variables for the stack"
echo "  - .env.cm    : Environment variables for cert-manager configuration"
echo ""
echo "Next Steps:"
echo "  - Load environment variables: source $SCRIPT_DIR/tmp/.env && source $SCRIPT_DIR/tmp/.env.cm"
echo "  - Access services (see .env or output from deploy-helm-charts.sh)"
echo "  - Issue certificates using cert-manager (Issuer: vault-issuer in namespace: ${CERT_MANAGER_NAMESPACE:-<Not Set>})"
echo ""
echo "Important Vault Information:"
echo "  - Vault ADDR        : ${VAULT_ADDR:-<Not Set>}"
echo "  - Vault Token       : ${VAULT_TOKEN:-<Not Set>} (retrieved from Kubernetes secret)"
echo ""
echo "Important cert-manager Information:"
echo "  - Namespace         : ${CERT_MANAGER_NAMESPACE:-<Not Set>}"
echo "  - Vault Issuer Name : ${VAULT_ISSUER_NAME:-vault-issuer}"
echo "  - Vault Cluster URL : ${CERT_MANAGER_VAULT_ADDR:-<Not Set>}"
echo ""
echo "Dashboard Access:"
echo "  - URL               : ${KUBERNETES_DASHBOARD_URL:-<Not Set>}"
echo "  - Token             : ${KUBERNETES_DASHBOARD_TOKEN:-<Not Set>}"
echo "=============================================="