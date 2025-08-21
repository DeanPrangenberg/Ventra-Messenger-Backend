#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BACKEND_ROOT_DIR="$SCRIPT_DIR/.."

source "$BACKEND_ROOT_DIR/scripts/functions/logs.sh"
source "$BACKEND_ROOT_DIR/scripts/functions/env.sh"

# Check if kubectl exists, if not install
if ! command -v kubectl &>/dev/null; then
  log "Installing kubectl..."
  curl -LO "https://dl.k8s.io/release/$(curl -L -s https://dl.k8s.io/release/stable.txt)/bin/linux/amd64/kubectl"
  sudo install -o root -g root -m 0755 kubectl /usr/local/bin/kubectl
  rm kubectl
else
  log "kubectl already installed."
fi

# Check if argocd CLI exists, if not install
if ! command -v argocd &>/dev/null; then
  log "Installing argocd CLI..."
  curl -sSL -o argocd https://github.com/argoproj/argo-cd/releases/latest/download/argocd-linux-amd64
  sudo install -m 555 argocd /usr/local/bin/argocd
  rm argocd
else
  log "argocd CLI already installed."
fi

# Check if Kubernetes cluster is running
if ! kubectl cluster-info &>/dev/null; then
  log "No Kubernetes cluster detected. Installing K3s..."

  # Install k3s
  curl -sfL https://get.k3s.io | sh -

  # Wait for k3s to be ready
  log_wait "Waiting for K3s to start..."
  export KUBECONFIG=/etc/rancher/k3s/k3s.yaml
  for i in {1..30}; do
    kubectl get nodes &>/dev/null && break
    log_wait "Waiting for Kubernetes cluster..."
    sleep 5
  done

  if ! kubectl get nodes &>/dev/null; then
    error "Failed to start K3s."
    exit 1
  fi

  log "K3s installed and running."
else
  log "Kubernetes cluster detected."
fi

# Create ArgoCD namespace if it doesn't exist
log "Setting up ArgoCD namespace..."
kubectl create namespace argocd 2>/dev/null || true

# Install ArgoCD if the deployment doesn't exist
if ! kubectl -n argocd get deployment argocd-server &>/dev/null; then
  log "Installing ArgoCD in cluster..."
  kubectl apply -n argocd -f https://raw.githubusercontent.com/argoproj/argo-cd/stable/manifests/install.yaml

  log "Verifying ArgoCD installation started..."
  sleep 10
  kubectl -n argocd get pods

  log_wait "Waiting for ArgoCD server deployment to be ready (this may take a few minutes)..."
  kubectl -n argocd wait --for=condition=available --timeout=300s deployment/argocd-server || {
    error "ArgoCD server deployment failed to become ready within timeout."
    kubectl -n argocd get pods
    exit 1
  }

  log "ArgoCD installed successfully."
else
  log "ArgoCD already installed."
fi

log "Patching ArgoCD"
kubectl patch svc argocd-server -n argocd -p '{"spec":{"type":"NodePort","ports":[{"name":"http","port":80,"protocol":"TCP","targetPort":8080,"nodePort":30200}]}}'

log_wait "Waiting for ArgoCD CRDs to be established..."
for i in {1..30}; do
  if kubectl get crd applications.argoproj.io &>/dev/null; then
    log "ArgoCD Application CRD is available."
    break
  fi
  log_wait "Waiting for CRDs (attempt $i/30)..."
  sleep 5
  if [ $i -eq 30 ]; then
    error "Timed out waiting for ArgoCD CRDs."
    exit 1
  fi
done

# Deployment options
APPS_DIRS=(
  "single-node-dev"
  "single-node-prod"
  "single-node-test"
  "multi-node-dev"
  "multi-node-prod"
  "multi-node-test"
)

APPS=(
  "Single Node Development"
  "Single Node Production (Not ready)"
  "Single Node Test (Not ready)"
  "Multi Node Development (Not ready)"
  "Multi Node Production (Not ready)"
  "Multi Node Test (Not ready)"
)

log "Select the deployment to apply:"
for i in "${!APPS[@]}"; do
  log_user "$((i+1))) ${APPS[i]}"
done

read -rp "Enter your choice (1-6): " choice
if [[ "$choice" -lt 1 || "$choice" -gt 6 ]]; then
  error "Invalid choice."
  exit 1
fi

APP="${APPS_DIRS[choice-1]}"
APP_FILE="argocd-apps/${APP}.yaml"

if [[ ! -f "$APP_FILE" ]]; then
  error "Application manifest $APP_FILE not found."
  exit 1
fi

log "Deploying ArgoCD Application $APP..."
kubectl apply -f "$APP_FILE"

log "Done."
