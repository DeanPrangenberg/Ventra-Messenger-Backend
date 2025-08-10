#!/bin/bash

# Strict mode
set -euo pipefail

# --- Configuration ---
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BACKEND_ROOT_DIR="$SCRIPT_DIR/../../../.."
KAFKA_VALUES_FILE="$BACKEND_ROOT_DIR/.config/kubernetes/kafka/values.yaml"
KAFKA_STORAGE_CLASS_FILE="$BACKEND_ROOT_DIR/.config/kubernetes/storageClasses/kafka/local-kafka-storage.yaml"
KAFKA_PV_FILE="$BACKEND_ROOT_DIR/.config/kubernetes/PVCs/kafka/local-kafka-storage-pvc.yaml"

# Source shared functions (assuming they exist and work)
source "$BACKEND_ROOT_DIR/.scripts/functions/logs.sh"
source "$BACKEND_ROOT_DIR/.scripts/functions/env.sh"

KAFKA_NAMESPACE="kafka"
CERT_MANAGER_NAMESPACE="cert-manager"
RELEASE_NAME="my-kafka"
ISSUER_NAME="vault-issuer" # Use your existing Vault ClusterIssuer
BROKER_CERT_SECRET_NAME="kafka-server-tls"
CLIENT_CERT_SECRET_NAME="kafka-client-tls"

# --- Execution ---
# 1. Create Namespace
log "Creating Kafka namespace if it doesn't exist..."
kubectl create namespace ${KAFKA_NAMESPACE} || log "Namespace '${KAFKA_NAMESPACE}' already exists."

kubectl patch storageclass local-path -p '{"metadata": {"annotations":{"storageclass.kubernetes.io/is-default-class":"false"}}}'

log "Setting up Kafka storage..."
log "Checking for Kafka StorageClass..."
if ! kubectl get storageclass local-kafka-storage >/dev/null 2>&1; then
    log "Creating Kafka StorageClass..."
    kubectl apply -f "${KAFKA_STORAGE_CLASS_FILE}"
    log "Kafka StorageClass created."
else
    log "Kafka StorageClass 'local-kafka-storage' already exists. Skipping creation."
fi

log "Checking for Kafka PersistentVolume..."
if ! kubectl get pv local-pv-kafka-0 >/dev/null 2>&1; then
    log "Creating local directory and PersistentVolume for Kafka..."
    log_important_user "This will create a local storage directory (/mnt/kafka-data-0) and PV for Kafka. Ensure the node is accessible."
    if [ ! -d /mnt/kafka-data-0 ]; then
        log "Creating local directory /mnt/kafka-data-0 on the node..."
        sudo mkdir -p /mnt/kafka-data-0
        sudo chown 1001:1001 /mnt/kafka-data-0
        sudo chmod 777 /mnt/kafka-data-0
        log "Local directory /mnt/kafka-data-0 created and permissions set."
    else
        log "Local directory /mnt/kafka-data-0 already exists."
    fi

    NODE_NAME=$(kubectl get nodes -o jsonpath='{.items[0].metadata.name}')
    if [ -z "$NODE_NAME" ]; then
        log_error "Could not get a node name to create the Kafka PersistentVolume."
        exit 1
    fi
    log "Using node '${NODE_NAME}' for Kafka PersistentVolume."

    sed "s/__NODE_NAME__/${NODE_NAME}/g" "${KAFKA_PV_FILE}" | kubectl apply -f -
    log "Kafka PersistentVolume 'local-pv-kafka-0' created."
else
    log "Kafka PersistentVolume 'local-pv-kafka-0' already exists. Skipping creation."
fi

log "Kafka storage setup complete."

# 3. Request TLS Certificates
log "Requesting TLS certificates for Kafka..."
# Server Certificate
cat <<EOF | kubectl apply -n ${KAFKA_NAMESPACE} -f -
apiVersion: cert-manager.io/v1
kind: Certificate
metadata:
  name: kafka-broker-certificate
spec:
  secretName: ${BROKER_CERT_SECRET_NAME}
  issuerRef:
    name: ${ISSUER_NAME}
    kind: ClusterIssuer
    group: cert-manager.io
  commonName: ${RELEASE_NAME}-headless.${KAFKA_NAMESPACE}.svc.cluster.local
  dnsNames:
  - "*.${RELEASE_NAME}-controller-headless.${KAFKA_NAMESPACE}.svc.cluster.local" # Wildcard for controller pods
  - "${RELEASE_NAME}-controller-headless.${KAFKA_NAMESPACE}.svc.cluster.local"   # Controller headless service
  - "${RELEASE_NAME}-controller-0.${RELEASE_NAME}-controller-headless.${KAFKA_NAMESPACE}.svc.cluster.local" # Specific pod name
  - "${RELEASE_NAME}.${KAFKA_NAMESPACE}.svc.cluster.local"                       # Main service name
  - "localhost"
  - "127.0.0.1"
  privateKey:
    algorithm: RSA
    size: 4096
EOF

# Client Certificate
cat <<EOF | kubectl apply -n ${KAFKA_NAMESPACE} -f -
apiVersion: cert-manager.io/v1
kind: Certificate
metadata:
  name: kafka-client-certificate
spec:
  secretName: ${CLIENT_CERT_SECRET_NAME}
  issuerRef:
    name: ${ISSUER_NAME}
    kind: ClusterIssuer
    group: cert-manager.io
  commonName: "kafka-client"
  isCA: false
  privateKey:
    algorithm: RSA
    size: 4096
  usages:
    - client auth
EOF

log_wait "Waiting for Kafka certificates to be issued..."
kubectl wait --for=condition=Ready=True --timeout=120s \
  certificate/kafka-broker-certificate -n ${KAFKA_NAMESPACE}
kubectl wait --for=condition=Ready=True --timeout=120s \
  certificate/kafka-client-certificate -n ${KAFKA_NAMESPACE}
log "Kafka certificates issued successfully."

# 4. Install Kafka via Helm
log "Installing Kafka via Helm..."
helm repo add bitnami https://charts.bitnami.com/bitnami
helm repo update

# Cleanup any previous installation
log "Cleaning up any existing Kafka resources..."
helm uninstall ${RELEASE_NAME} --namespace ${KAFKA_NAMESPACE} || true
kubectl delete pvc -n ${KAFKA_NAMESPACE} --all || true
kubectl delete pod -n ${KAFKA_NAMESPACE} --all || true
sleep 10 # Allow cleanup

# Perform the installation with explicit overrides to ensure config is applied
log "Installing Kafka with explicit configuration..."
helm install my-kafka bitnami/kafka \
  --namespace kafka \
  --create-namespace \
  -f $BACKEND_ROOT_DIR/.config/kubernetes/kafka/values.yaml \
  --set persistence.storageClass=local-kafka-storage \
  --set persistence.annotations."volume\.beta\.kubernetes\.io/storage-class"=local-kafka-storage \
  --set auth.tls.existingSecret=kafka-server-tls \
  --set readinessProbe.initialDelaySeconds=120 \
  --set readinessProbe.failureThreshold=20 \
  --set livenessProbe.initialDelaySeconds=180 \
  --timeout 15m

log_wait "Waiting for Kafka Pod to be ready..."
# Wait specifically for the pod to be ready
kubectl wait --for=condition=ready pod my-kafka-controller-0 -n ${KAFKA_NAMESPACE} --timeout=600s

log "Kafka installation complete."

# 5. Test Kafka Functionality
log "Testing Kafka installation..."
sleep 30 # Give Kafka a moment after the pod is ready

# List topics to verify basic operation
log "Listing Kafka topics..."
if kubectl exec -n ${KAFKA_NAMESPACE} my-kafka-controller-0 -- \
   kafka-topics.sh --bootstrap-server my-kafka-controller-0.my-kafka-controller-headless.kafka.svc.cluster.local:9092 \
   --command-config /config/client.properties --list; then
    log "Topics listed successfully."
else
    log "WARNING: Could not list topics, but installation might still be successful."
fi

# Create a test topic
log "Creating test topic..."
kubectl exec -n ${KAFKA_NAMESPACE} my-kafka-controller-0 -- \
kafka-topics.sh --bootstrap-server my-kafka-controller-0.my-kafka-controller-headless.kafka.svc.cluster.local:9092 \
--command-config /config/client.properties --create --topic test-topic --partitions 3 --replication-factor 1 || true

# Send a test message
log "Sending test message..."
echo "Test_Message_From_Script_mTLS" | kubectl exec -i -n ${KAFKA_NAMESPACE} my-kafka-controller-0 -- \
kafka-console-producer.sh --bootstrap-server my-kafka-controller-0.my-kafka-controller-headless.kafka.svc.cluster.local:9092 \
--producer.config /config/client.properties --topic test-topic

# Consume the test message
log "Receiving test message..."
kubectl exec -n ${KAFKA_NAMESPACE} my-kafka-controller-0 -- \
kafka-console-consumer.sh --bootstrap-server my-kafka-controller-0.my-kafka-controller-headless.kafka.svc.cluster.local:9092 \
--consumer.config /config/client.properties --topic test-topic --from-beginning --max-messages 1 --timeout-ms 10000

log "Kafka (Single-Node KRaft with mTLS) installation and testing complete."
