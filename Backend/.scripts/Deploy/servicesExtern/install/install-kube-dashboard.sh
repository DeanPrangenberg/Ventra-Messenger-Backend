#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BACKEND_ROOT_DIR="$SCRIPT_DIR/../../../.."
DASHBOARD_TOKEN_FILE=$BACKEND_ROOT_DIR/.data/other/dashboard/dashboard-token.txt
DASHBOARD_URL_FILE=$BACKEND_ROOT_DIR/.data/other/dashboard/dashboard-url.txt

# Source shared functions
source "$BACKEND_ROOT_DIR/.scripts/functions/env.sh"
source "$BACKEND_ROOT_DIR/.scripts/functions/logs.sh"

DASHBOARD_NS="dashboard"

log "Creating Dashboard namespace if it doesn't exist..."
kubectl create namespace "$DASHBOARD_NS" 2>/dev/null || log "Dashboard namespace already exists."
log "Dashboard namespace is ready."

log "Installing Kubernetes Dashboard..."
curl -sL https://raw.githubusercontent.com/kubernetes/dashboard/v2.7.0/aio/deploy/recommended.yaml |
  sed -e "s/namespace: kubernetes-dashboard/namespace: $DASHBOARD_NS/g" \
      -e "s/--namespace=kubernetes-dashboard/--namespace=$DASHBOARD_NS/g" |
  kubectl apply -f - >/dev/null 2>&1

# Kurz warten
sleep 2

# Admin User erstellen
log "Creating Dashboard admin user..."
kubectl create serviceaccount admin-user -n "$DASHBOARD_NS" 2>/dev/null || true
kubectl create clusterrolebinding admin-user --clusterrole=cluster-admin --serviceaccount="$DASHBOARD_NS":admin-user 2>/dev/null || true

# NodePort Service
log "Creating NodePort service for Dashboard..."
cat <<EOF | kubectl apply -f - >/dev/null 2>&1
apiVersion: v1
kind: Service
metadata:
  name: kubernetes-dashboard-nodeport
  namespace: $DASHBOARD_NS
spec:
  type: NodePort
  ports:
    - port: 443
      targetPort: 8443
      nodePort: 30443
  selector:
    k8s-app: kubernetes-dashboard
EOF

# 8. Dashboard Token generieren
log "Generating Dashboard login token..."
DASHBOARD_TOKEN=$(kubectl -n "$DASHBOARD_NS" create token admin-user --duration=8760h 2>/dev/null || echo "Token generation failed")

if [[ "$DASHBOARD_TOKEN" == "Token generation failed" || -z "$DASHBOARD_TOKEN" ]]; then
  log_warn "Could not generate Dashboard token automatically"
  log_warn "You can get it manually later by running:"
  log_warn "kubectl -n $DASHBOARD_NS create token admin-user"
  DASHBOARD_TOKEN="<Token generation failed - see instructions above>"
else
  log "Dashboard token generated successfully"
fi

mkdir -p "$(dirname "$DASHBOARD_TOKEN_FILE")"
echo "$DASHBOARD_TOKEN" >"$DASHBOARD_TOKEN_FILE"
echo "https://$(hostname -I | awk '{print $1}'):30443" >"$DASHBOARD_URL_FILE"