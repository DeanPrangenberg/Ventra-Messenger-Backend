#!/bin/bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BACKEND_ROOT_DIR="$SCRIPT_DIR/../.."
echo "[DEBUG] Script directory is: $SCRIPT_DIR"
echo "[DEBUG] Backend root directory is: $BACKEND_ROOT_DIR"

source "$BACKEND_ROOT_DIR/.scripts/functions/logs.sh"
source "$BACKEND_ROOT_DIR/.scripts/functions/env.sh"

# 1. Check prerequisites: Exactly one argument (namespace) is required
if [[ $# -ne 1 ]]; then
    error "Target Kubernetes namespace is required."
    error "Usage: $0 <namespace>"
    exit 1
fi

NAMESPACE="$1" # Set the namespace from the first argument

# 2. Load environment variables
source_env_file "$BACKEND_ROOT_DIR/.env"

# 3. Check if required environment variables are set
if [[ -z "${GITHUB_USERNAME:-}" ]] || [[ -z "${GITHUB_IMAGE_TOKEN:-}" ]]; then
    error "GITHUB_USERNAME or GITHUB_IMAGE_TOKEN not set in .env file. Cannot create image pull secret."
    exit 1
fi

# 4. Create or update the Kubernetes Secret
log "Creating/updating Kubernetes image pull secret 'ventra-ghcr-secret' in namespace '$NAMESPACE'..."
if kubectl create secret docker-registry ventra-ghcr-secret \
  --namespace "$NAMESPACE" \
  --docker-server=ghcr.io \
  --docker-username="${GITHUB_USERNAME,,}" \
  --docker-password="$GITHUB_IMAGE_TOKEN" \
  --docker-email="noreply@example.com" \
  --dry-run=client -o yaml | kubectl apply -f -; then
    log "Kubernetes image pull secret 'ventra-ghcr-secret' created/updated successfully in namespace '$NAMESPACE'."
else

    error "Failed to create/update Kubernetes image pull secret 'ventra-ghcr-secret' in namespace '$NAMESPACE'."
    exit 1
fi