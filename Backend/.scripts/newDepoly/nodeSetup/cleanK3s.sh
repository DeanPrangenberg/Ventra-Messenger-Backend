#!/bin/bash
set -euo pipefail

# escalate if needed
if [ "$(id -u)" -ne 0 ]; then
  exec sudo --preserve-env=KUBECONFIG "$0" "$@"
fi

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BACKEND_ROOT_DIR="$SCRIPT_DIR/../../.."
echo "[DEBUG] Script directory is: $SCRIPT_DIR"
echo "[DEBUG] Backend root directory is: $BACKEND_ROOT_DIR"

source "$BACKEND_ROOT_DIR/.scripts/functions/logs.sh"
source "$BACKEND_ROOT_DIR/.scripts/functions/env.sh"

log_important_user "This will WIPE your entire K3s cluster. Are you sure? (y/N)"
read -r confirm
if [ "$confirm" != "y" ]; then
  log "Aborted."
  exit 0
fi

# === 1. Full k3s wipe ===
echo "=== Wiping k3s installation ==="
if command -v /usr/local/bin/k3s-killall.sh >/dev/null 2>&1; then
  /usr/local/bin/k3s-killall.sh || true
fi
if command -v systemctl >/dev/null 2>&1; then
  systemctl disable k3s || true
  systemctl reset-failed k3s || true
fi

rm -rf /etc/systemd/system/k3s* \
       /etc/rancher/k3s \
       /run/k3s \
       /run/flannel \
       /var/lib/kubelet \
       /var/lib/rancher/k3s \
       /mnt/vault-data-0

rm -f /usr/local/bin/k3s /usr/local/bin/k3s-killall.sh \
      /usr/local/bin/kubectl /usr/local/bin/crictl /usr/local/bin/ctr

# === 2. Reinstall k3s ===
log "=== Installing k3s fresh ==="
# write-kubeconfig-mode=644 so non-root can read it
curl -sfL https://get.k3s.io | INSTALL_K3S_EXEC="--write-kubeconfig-mode=644" sh -

# === 3. Setup kubeconfig for current user ===
K3S_KUBECONFIG=/etc/rancher/k3s/k3s.yaml
# ensure the file is readable by non-root (should already be 644 if install honored it)
chmod 644 "$K3S_KUBECONFIG"

# if the script was invoked via sudo, $SUDO_USER is the real user; otherwise fallback to current
TARGET_USER=${SUDO_USER:-$(logname 2>/dev/null || whoami)}
USER_HOME=$(eval echo "~$TARGET_USER")
USER_KUBEDIR="$USER_HOME/.kube"
USER_KUBECONFIG="$USER_KUBEDIR/config"

mkdir -p "$USER_KUBEDIR"
cp "$K3S_KUBECONFIG" "$USER_KUBECONFIG"
chown "$TARGET_USER":"$TARGET_USER" "$USER_KUBEDIR" "$USER_KUBECONFIG"
chmod 600 "$USER_KUBECONFIG"

export KUBECONFIG="$USER_KUBECONFIG"

# === 4. Wait for API ===
echo "Waiting for k3s API server to become ready..."
for i in {1..12}; do
  if kubectl get --raw=/healthz >/dev/null 2>&1; then
    log "API server is reachable."
    break
  fi
  log_warn "Not ready yet (attempt $i), waiting 5s..."
  sleep 5
done

# final check
if ! kubectl get nodes >/dev/null 2>&1; then
  error "k3s API server is not reachable. Logs:"
  journalctl -u k3s -n 100 --no-pager
  exit 1
fi

log "k3s is running. Nodes:"
kubectl get nodes

log "Done. You can now continue with your deployment steps."
