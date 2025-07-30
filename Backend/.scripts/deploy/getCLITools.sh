#!/bin/bash
# getCLITools.sh - Cross-distro CLI tool installer for Ventra stack

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BACKEND_ROOT_DIR="$SCRIPT_DIR/../.."
echo "[DEBUG] Script directory is: $SCRIPT_DIR"
echo "[DEBUG] Backend root directory is: $BACKEND_ROOT_DIR"

source "$BACKEND_ROOT_DIR/.scripts/functions/logs.sh"
source "$BACKEND_ROOT_DIR/.scripts/functions/env.sh"

TOOLS=(yq openssl jq curl kubectl vault helm docker)
MISSING=()

# Detect package manager
detect_pm() {
    if command -v apt-get &>/dev/null; then echo "apt"; return; fi
    if command -v dnf &>/dev/null; then echo "dnf"; return; fi
    if command -v yum &>/dev/null; then echo "yum"; return; fi
    if command -v zypper &>/dev/null; then echo "zypper"; return; fi
    if command -v pacman &>/dev/null; then echo "pacman"; return; fi
    log_error "No supported package manager found!"; exit 1
}

PM=$(detect_pm)
SUDO=""
if [[ $EUID -ne 0 ]]; then SUDO="sudo"; fi

# Check for missing tools
for tool in "${TOOLS[@]}"; do
    if ! command -v "$tool" &>/dev/null; then
        MISSING+=("$tool")
    fi
done

if [[ ${#MISSING[@]} -eq 0 ]]; then
    log "All required CLI tools are already installed."
    exit 0
fi

log "Missing tools: ${MISSING[*]}"
log "Installing missing tools using $PM..."

for tool in "${MISSING[@]}"; do
    case "$tool" in
        yq)
            if [[ "$PM" == "apt" ]]; then
                $SUDO apt-get update && $SUDO apt-get install -y yq || true
            elif [[ "$PM" == "dnf" || "$PM" == "yum" ]]; then
                $SUDO $PM install -y yq || true
            elif [[ "$PM" == "zypper" ]]; then
                $SUDO zypper install -y yq || true
            elif [[ "$PM" == "pacman" ]]; then
                $SUDO pacman -Sy --noconfirm yq || true
            fi
            if ! command -v yq &>/dev/null; then
                $SUDO wget -qO /usr/local/bin/yq https://github.com/mikefarah/yq/releases/latest/download/yq_linux_amd64
                $SUDO chmod +x /usr/local/bin/yq
            fi
            log "Installed yq"
            ;;
        openssl|jq|curl)
            if [[ "$PM" == "apt" ]]; then
                $SUDO apt-get update && $SUDO apt-get install -y "$tool"
            elif [[ "$PM" == "dnf" || "$PM" == "yum" ]]; then
                $SUDO $PM install -y "$tool"
            elif [[ "$PM" == "zypper" ]]; then
                $SUDO zypper install -y "$tool"
            elif [[ "$PM" == "pacman" ]]; then
                $SUDO pacman -Sy --noconfirm "$tool"
            fi
            log "Installed $tool"
            ;;
        kubectl)
            KURL="https://dl.k8s.io/release/$(curl -sL https://dl.k8s.io/release/stable.txt)/bin/linux/amd64/kubectl"
            $SUDO curl -sLO "$KURL"
            $SUDO install -m 0755 kubectl /usr/local/bin/kubectl
            rm -f kubectl
            log "Installed kubectl"
            ;;
        vault)
            VAULT_VERSION="1.16.2"
            VAULT_ZIP="vault_${VAULT_VERSION}_linux_amd64.zip"
            TMP_DIR="$(mktemp -d)"
            cd "$TMP_DIR"
            $SUDO mkdir -p /usr/local/bin
            $SUDO curl -sSLO "https://releases.hashicorp.com/vault/${VAULT_VERSION}/${VAULT_ZIP}"
            $SUDO unzip "$VAULT_ZIP"
            $SUDO mv vault /usr/local/bin/
            $SUDO chmod +x /usr/local/bin/vault
            cd -
            rm -rf "$TMP_DIR"
            log "Installed vault"
            ;;
        helm)
            curl -s https://raw.githubusercontent.com/helm/helm/main/scripts/get-helm-3 | $SUDO bash
            log "Installed helm"
            ;;
        docker)
            if [[ "$PM" == "apt" ]]; then
                $SUDO apt-get update
                $SUDO apt-get install -y ca-certificates curl gnupg lsb-release
                $SUDO mkdir -p /etc/apt/keyrings
                curl -fsSL https://download.docker.com/linux/ubuntu/gpg | $SUDO gpg --dearmor -o /etc/apt/keyrings/docker.gpg
                echo \
                  "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/docker.gpg] https://download.docker.com/linux/ubuntu \
                  $(lsb_release -cs) stable" | $SUDO tee /etc/apt/sources.list.d/docker.list > /dev/null
                $SUDO apt-get update
                $SUDO apt-get install -y docker-ce docker-ce-cli containerd.io
            elif [[ "$PM" == "dnf" || "$PM" == "yum" ]]; then
                $SUDO $PM install -y dnf-plugins-core
                $SUDO $PM config-manager --add-repo https://download.docker.com/linux/centos/docker-ce.repo
                $SUDO $PM install -y docker-ce docker-ce-cli containerd.io
                $SUDO systemctl enable --now docker
            elif [[ "$PM" == "zypper" ]]; then
                $SUDO zypper install -y docker
                $SUDO systemctl enable --now docker
            elif [[ "$PM" == "pacman" ]]; then
                $SUDO pacman -Sy --noconfirm docker
                $SUDO systemctl enable --now docker
            else
                log_warn "Automatic Docker installation not supported for this package manager. Please install Docker manually."
            fi
            log "Installed docker"
            ;;
        *)
            log_warn "Unknown tool: $tool. Please install manually."
            ;;
    esac
done

log "All required CLI tools are installed."