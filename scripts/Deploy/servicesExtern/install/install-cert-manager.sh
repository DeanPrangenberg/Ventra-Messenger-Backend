#!/bin/bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BACKEND_ROOT_DIR="$SCRIPT_DIR/../../../.."
VAULT_OTHER_DATA_DIR=$BACKEND_ROOT_DIR/.data/other/vault
VAULT_TMP_DATA_DIR=$BACKEND_ROOT_DIR/.data/tmp/vault
CA_CERT_FILE=$VAULT_OTHER_DATA_DIR/ca-cert.pem

source "$BACKEND_ROOT_DIR/scripts/functions/logs.sh"
source "$BACKEND_ROOT_DIR/scripts/functions/env.sh"

CERT_MANAGER_NAMESPACE="cert-manager"
CERT_MANAGER_VERSION="v1.13.3"

# 1. Install/Upgrade cert-manager CRDs first
log "Applying cert-manager CRDs..."
kubectl apply -f "https://github.com/cert-manager/cert-manager/releases/download/${CERT_MANAGER_VERSION}/cert-manager.crds.yaml"

# 2. Install cert-manager via Helm
log "Adding Jetstack Helm repository..."
helm repo add jetstack https://charts.jetstack.io
helm repo update

log "Installing cert-manager..."
helm install \
  cert-manager jetstack/cert-manager \
  --namespace "${CERT_MANAGER_NAMESPACE}" \
  --create-namespace \
  --version ${CERT_MANAGER_VERSION} \
  --set installCRDs=false # CRDs are installed manually above

log_wait "Waiting for cert-manager webhook to be ready..."
kubectl wait --for=condition=available --timeout=600s deployment/cert-manager-webhook -n "${CERT_MANAGER_NAMESPACE}"

log "Applying ClusterRole for cert-manager to create ServiceAccount tokens..."
cat <<EOF | kubectl apply -f -
apiVersion: rbac.authorization.k8s.io/v1
kind: ClusterRole
metadata:
  name: cert-manager-controller-vault-issuer
rules:
- apiGroups: [""]
  resources: ["serviceaccounts/token"]
  verbs: ["create"]
---
apiVersion: rbac.authorization.k8s.io/v1
kind: ClusterRoleBinding
metadata:
  name: cert-manager-controller-vault-issuer
roleRef:
  apiGroup: rbac.authorization.k8s.io
  kind: ClusterRole
  name: cert-manager-controller-vault-issuer
subjects:
- kind: ServiceAccount
  name: cert-manager
  namespace: cert-manager
EOF

ISSUER_NAME="vault-issuer"
VAULT_SERVER="http://pki-vault.vault.svc.cluster.local:8200"
VAULT_PATH="pki/sign/cert-manager"
VAULT_ROLE="cert-manager"
TLS_SECRET_NAME="vault-ca-secret"

# 3. ServiceAccount anlegen
log "Creating ServiceAccount for Vault Issuer..."
kubectl apply -n "${CERT_MANAGER_NAMESPACE}" -f - <<EOF
apiVersion: v1
kind: ServiceAccount
metadata:
  name: ${ISSUER_NAME}
  namespace: ${CERT_MANAGER_NAMESPACE}
EOF

# 4. TLS Secret nur mit der Vault CA anlegen
log "Creating TLS secret for Vault CA..."
if [ ! -f "$CA_CERT_FILE" ]; then
    log_error "Vault CA file not found at $CA_CERT_FILE"
    exit 1
fi
kubectl create secret generic ${TLS_SECRET_NAME} \
  --namespace ${CERT_MANAGER_NAMESPACE} \
  --from-file=ca.crt=${CA_CERT_FILE} \
  --dry-run=client -o yaml | kubectl apply -f -

# 5. cert-manager Issuer YAML bereitstellen und anwenden
log "Creating cert-manager Vault Issuer with enhanced mTLS support..."
cat <<EOF | kubectl apply -n ${CERT_MANAGER_NAMESPACE} -f -
apiVersion: cert-manager.io/v1
kind: ClusterIssuer
metadata:
  name: ${ISSUER_NAME}
spec:
  vault:
    server: ${VAULT_SERVER}
    path: ${VAULT_PATH}
    caBundle: $(base64 -w 0 < ${CA_CERT_FILE})
    auth:
      kubernetes:
        mountPath: /v1/auth/kubernetes
        role: ${VAULT_ROLE}
        serviceAccountRef:
          name: ${ISSUER_NAME}
EOF

log "cert-manager Issuer '${ISSUER_NAME}' created in namespace '${CERT_MANAGER_NAMESPACE}' with CA bundle for mTLS."

# 6. Additional ClusterIssuer for internal service communication
log "Creating additional ClusterIssuer for internal mTLS..."
cat <<EOF | kubectl apply -f -
apiVersion: cert-manager.io/v1
kind: ClusterIssuer
metadata:
  name: vault-issuer-internal
spec:
  vault:
    server: ${VAULT_SERVER}
    path: pki/sign/internal-services
    caBundle: $(base64 -w 0 < ${CA_CERT_FILE})
    auth:
      kubernetes:
        mountPath: /v1/auth/kubernetes
        role: ${VAULT_ROLE}
        serviceAccountRef:
          name: ${ISSUER_NAME}
EOF

log "cert-manager additional Issuer 'vault-issuer-internal' created for internal services in namespace '${CERT_MANAGER_NAMESPACE}'."
