#!/bin/bash
# cert-manager.sh - Configures cert-manager to use Vault with a pre-created token

CERT_MANAGER_NAMESPACE="${CERT_MANAGER_NAMESPACE:-cert-manager}"
VAULT_TOKEN_SECRET="vault-cert-manager-token"
VAULT_SECRET_NAMESPACE="${VAULT_SECRET_NAMESPACE:-vault}"
VAULT_ADDR="${VAULT_ADDR:-http://127.0.0.1:30443}"
INTERMEDIATE_PKI_PATH="${INTERMEDIATE_PKI_PATH:-pki_int}"
SERVICE_CERT_ROLE="${SERVICE_CERT_ROLE:-internal-services}"
LOG_LEVEL="${LOG_LEVEL:-INFO}"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BACKEND_ROOT_DIR="$SCRIPT_DIR/../../.."
source "$BACKEND_ROOT_DIR/.scripts/functions/logs.sh"

set -eE

fetch_vault_token_from_secret() {
    log "Fetching Vault token from Kubernetes secret '$VAULT_TOKEN_SECRET' in namespace '$VAULT_SECRET_NAMESPACE'..."
    VAULT_TOKEN=$(kubectl get secret "$VAULT_TOKEN_SECRET" --namespace "$VAULT_SECRET_NAMESPACE" -o jsonpath="{.data.token}" | base64 -d)
    if [[ -z "$VAULT_TOKEN" ]]; then
        error "Failed to fetch Vault token from secret"
        exit 1
    fi
    export VAULT_TOKEN
    log "Vault token loaded from Kubernetes secret"
}

check_prerequisites() {
    log "Checking prerequisites..."
    command -v kubectl >/dev/null || error "kubectl is not installed"
    command -v vault >/dev/null || error "vault CLI is not installed"
    kubectl cluster-info >/dev/null || error "No connection to Kubernetes cluster"
    [[ -z "$VAULT_TOKEN" ]] && error "VAULT_TOKEN must be set"
    export VAULT_ADDR="$VAULT_ADDR"
    export VAULT_TOKEN="$VAULT_TOKEN"
    vault status >/dev/null || error "No connection to Vault at $VAULT_ADDR"
    log "All prerequisites met"
}

create_vault_issuer() {
    log "Creating VaultIssuer for cert-manager..."
    vault read -field=certificate "$INTERMEDIATE_PKI_PATH/cert/ca" > $BACKEND_ROOT_DIR/.scripts/deploy/tmp/cert_manager.cert.pem
    local ca_bundle
    ca_bundle=$(base64 -w 0 $BACKEND_ROOT_DIR/.scripts/deploy/tmp/cert_manager.cert.pem)
    local cluster_vault_addr="$VAULT_ADDR"
    cat <<EOF | kubectl apply -f -
apiVersion: cert-manager.io/v1
kind: Issuer
metadata:
  name: vault-issuer
  namespace: $CERT_MANAGER_NAMESPACE
spec:
  vault:
    server: $cluster_vault_addr
    path: $INTERMEDIATE_PKI_PATH/sign/$SERVICE_CERT_ROLE
    caBundle: $ca_bundle
    auth:
      tokenSecretRef:
        name: $VAULT_TOKEN_SECRET
        key: token
EOF
    log "VaultIssuer 'vault-issuer' in namespace '$CERT_MANAGER_NAMESPACE' created"
}

# Main execution
fetch_vault_token_from_secret
check_prerequisites
create_vault_issuer