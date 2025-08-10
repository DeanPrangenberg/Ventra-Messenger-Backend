#!/bin/bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BACKEND_ROOT_DIR="$SCRIPT_DIR/../../.."
echo "[DEBUG] Script directory is: $SCRIPT_DIR"
echo "[DEBUG] Backend root directory is: $BACKEND_ROOT_DIR"

source "$BACKEND_ROOT_DIR/scripts/functions/logs.sh"
source "$BACKEND_ROOT_DIR/scripts/functions/env.sh"

declare -A IMAGE_PATH_MAP=(
    ["$BACKEND_ROOT_DIR/init-kafka/Dockerfile"]="init-kafka"
    ["$BACKEND_ROOT_DIR/init-vault/Dockerfile"]="init-vault"
    ["$BACKEND_ROOT_DIR/VM-API/Dockerfile"]="vm-api"
    ["$BACKEND_ROOT_DIR/VM-AUTH/Dockerfile"]="vm-auth"
    ["$BACKEND_ROOT_DIR/VM-CORE/Dockerfile"]="vm-core"
    ["$BACKEND_ROOT_DIR/VM-LOGGER/Dockerfile"]="vm-logger"
    ["$BACKEND_ROOT_DIR/VM-MD/Dockerfile"]="vm-md"
    ["$BACKEND_ROOT_DIR/VM-REDIS-API/Dockerfile"]="vm-redis-api"
)

log "Starting Docker image build and push process..."

CLEANUP_LOCAL_IMAGES="no"
echo ""
read -p "Do you want to delete the local Docker images after pushing them? (y/N): " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    CLEANUP_LOCAL_IMAGES="yes"
    log "Local image cleanup is enabled."
else
    log "Local image cleanup is disabled."
fi
echo ""

command -v docker >/dev/null 2>&1 || error "Docker is not installed or not in PATH."

ROOT_ENV_FILE="$BACKEND_ROOT_DIR/.env"
source_env_file "$ROOT_ENV_FILE"

# --- KORREKTUR: Variablen laden und verarbeiten ---
# 1. Prüfen, ob die Variablen gesetzt sind (Originalwerte)
if [[ -z "${GITHUB_IMAGE_TOKEN:-}" ]] || [[ -z "${GITHUB_USERNAME:-}" ]]; then
    error "GITHUB_IMAGE_TOKEN or GITHUB_USERNAME not set in $ROOT_ENV_FILE."
fi

# 2. Konvertiere GITHUB_USERNAME EINMAL in Kleinbuchstaben für die weitere Verwendung
GITHUB_USERNAME_LOWERCASE="${GITHUB_USERNAME,,}"

# 3. Verwende die konvertierte Variable für Login und Image-Namen
log "Logging into GitHub Container Registry (GHCR)..."
echo "$GITHUB_IMAGE_TOKEN" | docker login ghcr.io -u "$GITHUB_USERNAME_LOWERCASE" --password-stdin || error "Failed to log in to GHCR."

# Store the original directory to return to it later
ORIGINAL_DIR="$PWD"

for dockerfile_path in "${!IMAGE_PATH_MAP[@]}"; do
    image_name="${IMAGE_PATH_MAP[$dockerfile_path]}"
    local_image_tag="${image_name}:latest"
    ghcr_image_name="ghcr.io/${GITHUB_USERNAME_LOWERCASE}/ventra-${image_name}"
    ghcr_image_tag="${ghcr_image_name}:latest"

    log "Processing image: $image_name"

    if [[ ! -f "$dockerfile_path" ]]; then
        error "Dockerfile not found: $dockerfile_path"
    fi

    # Determine the relative path of the Dockerfile within the build context (BACKEND_ROOT_DIR)
    relative_dockerfile_path="${dockerfile_path#$BACKEND_ROOT_DIR/}"

    if [[ "$relative_dockerfile_path" == "$dockerfile_path" ]]; then
        error "Could not determine relative path for Dockerfile: $dockerfile_path. Check BACKEND_ROOT_DIR."
    fi

    log "Building image $local_image_tag..."
    # Change to the backend root directory to ensure the build context is correct
    # Then run docker build with the relative path to the Dockerfile and '.' as context
    if (cd "$BACKEND_ROOT_DIR" && docker build -f "$relative_dockerfile_path" -t "$local_image_tag" .); then
        log "Successfully built $local_image_tag"
    else
        error "Failed to build image $image_name"
    fi

    log "Tagging image for GHCR..."
    if docker tag "$local_image_tag" "$ghcr_image_tag"; then
        log "Successfully tagged $ghcr_image_tag"
    else
        error "Failed to tag image $image_name for GHCR"
    fi

    log "Pushing image to GHCR..."
    if docker push "$ghcr_image_tag"; then
        log "Successfully pushed $ghcr_image_tag"
    else
        error "Failed to push image $ghcr_image_tag to GHCR"
    fi

    if [[ "$CLEANUP_LOCAL_IMAGES" == "yes" ]]; then
        log "Removing local images..."
        docker rmi "$local_image_tag" "$ghcr_image_tag" 2>/dev/null || log_warn "Could not remove one or more local image tags."
    fi

    echo ""
done

# Return to the original directory
cd "$ORIGINAL_DIR" || log_warn "Could not return to the original directory."

log "All Docker images built and pushed to GHCR successfully!"

if [[ "$CLEANUP_LOCAL_IMAGES" == "yes" ]]; then
    log "Performing final cleanup..."
    for dockerfile_path in "${!IMAGE_PATH_MAP[@]}"; do
        image_name="${IMAGE_PATH_MAP[$dockerfile_path]}"
        local_image_tag="${image_name}:latest"
        # Verwende die bereits konvertierte Variable
        ghcr_image_name="ghcr.io/${GITHUB_USERNAME_LOWERCASE}/ventra-${image_name}"
        ghcr_image_tag="${ghcr_image_name}:latest"

        docker rmi "$local_image_tag" "$ghcr_image_tag" 2>/dev/null || true
    done
    log "Final cleanup completed."
fi

log "Docker image build and push process completed."