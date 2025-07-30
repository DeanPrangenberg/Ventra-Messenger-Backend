#!/bin/bash
# deploy-helm-charts.sh - Deploys the Ventra stack with Helm charts for k3s

echo "Fast Ventra Stack Installation for k3s"
echo "======================================"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BACKEND_ROOT_DIR="$SCRIPT_DIR/../.."
echo "[DEBUG] Script directory is: $SCRIPT_DIR"
echo "[DEBUG] Backend root directory is: $BACKEND_ROOT_DIR"

source "$BACKEND_ROOT_DIR/.scripts/functions/logs.sh"
source "$BACKEND_ROOT_DIR/.scripts/functions/env.sh"

# 1. SCRIPT_DIR und Pfade definieren (Ganz früh im Skript!)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
log "[DEBUG] Script directory is: $SCRIPT_DIR"

# Pfade zu den Konfigurationsdateien definieren
KUBE_CONFIGS="$SCRIPT_DIR/../../.kubeConfig" # Korrigiert: Relativer Pfad vom Skript-Verzeichnis

# 2. Prüfen ob k3s läuft
log "Checking if k3s is running..."
if ! systemctl is-active --quiet k3s; then
    log_error "k3s is not running! Please start k3s first:"
    log_error "sudo systemctl start k3s"
    exit 1
fi

# 3. Helm Repositories hinzufügen
log "Adding Helm repositories..."
helm repo add hashicorp https://helm.releases.hashicorp.com   2>/dev/null || true
helm repo add bitnami https://charts.bitnami.com/bitnami   2>/dev/null || true
helm repo add prometheus-community https://prometheus-community.github.io/helm-charts   2>/dev/null || true
helm repo add grafana https://grafana.github.io/helm-charts   2>/dev/null || true
helm repo add jetstack https://charts.jetstack.io   2>/dev/null || true # Für cert-manager

log "Updating Helm repositories..."
helm repo update > /dev/null 2>&1

# 4. Namespaces erstellen
log "Creating namespaces..."
kubectl create namespace vault 2>/dev/null || true
kubectl create namespace kafka 2>/dev/null || true
kubectl create namespace database 2>/dev/null || true
kubectl create namespace redis 2>/dev/null || true
kubectl create namespace monitoring 2>/dev/null || true
kubectl create namespace kubernetes-dashboard 2>/dev/null || true
kubectl create namespace cert-manager 2>/dev/null || true # Für cert-manager

# 5. Alle Helm Services installieren
log "Installing all Helm charts..."

log "Installing Vault..."
# Pfad zur Vault Values-Datei korrigieren (extern)
VAULT_VALUES_FILE="$KUBE_CONFIGS/extern/vault-values.yaml"

# Überprüfen, ob die Datei existiert
if [[ ! -f "$VAULT_VALUES_FILE" ]]; then
    log_error "Vault values file not found: $VAULT_VALUES_FILE"
    log_error "Please ensure the file exists at the specified path."
    exit 1
fi

helm upgrade --install vault hashicorp/vault \
  --namespace vault \
  --values "$VAULT_VALUES_FILE"

log "Patching Vault service to NodePort..."
kubectl patch svc vault -n vault -p '{"spec": {"type": "NodePort", "ports": [{"name": "http", "port": 8200, "targetPort": 8200, "nodePort": 30200}]}}'

log "Installing Kafka..."
helm upgrade --install kafka bitnami/kafka \
  --namespace kafka \
  --set zookeeper.enabled=true \
  --set replicaCount=1 \
  --set offsetsTopicReplicationFactor=1 \
  --set service.ports.client=9092 \
  --set externalAccess.enabled=true \
  --set externalAccess.service.type=NodePort \
  --set externalAccess.service.nodePorts.external=30092 > /dev/null 2>&1

log "Installing PostgreSQL..."
helm upgrade --install postgres bitnami/postgresql \
  --namespace database \
  --set auth.postgresPassword="testPassword1234" \
  --set auth.username="dev" \
  --set auth.database="main_db" \
  --set primary.service.ports.postgresql=5432 \
  --set primary.persistence.enabled=false > /dev/null 2>&1

log "Installing Redis..."
helm upgrade --install redis bitnami/redis \
  --namespace redis \
  --set master.service.ports.redis=6379 \
  --set master.persistence.enabled=false \
  --set architecture=standalone > /dev/null 2>&1

log "Installing Monitoring Stack..."
helm upgrade --install prometheus prometheus-community/prometheus \
  --namespace monitoring \
  --set server.service.type=NodePort \
  --set server.service.nodePort=30090 > /dev/null 2>&1

helm upgrade --install grafana grafana/grafana \
  --namespace monitoring \
  --set adminUser=admin \
  --set adminPassword=admin \
  --set service.type=NodePort \
  --set service.nodePort=30000 > /dev/null 2>&1

# 6. cert-manager installieren
log "Installing cert-manager..."
# cert-manager installieren
helm upgrade --install cert-manager jetstack/cert-manager \
    --namespace cert-manager \
    --version v1.14.4 \
    --set installCRDs=true \
    --set prometheus.enabled=false > /dev/null 2>&1

log "Waiting for cert-manager to be ready..."
kubectl wait --for=condition=available --timeout=300s deployment/cert-manager -n cert-manager 2>/dev/null || true
kubectl wait --for=condition=available --timeout=300s deployment/cert-manager-webhook -n cert-manager 2>/dev/null || true
kubectl wait --for=condition=available --timeout=300s deployment/cert-manager-cainjector -n cert-manager 2>/dev/null || true
log "cert-manager installed"

# 7. Kubernetes Dashboard installieren
log "Installing Kubernetes Dashboard..."
kubectl apply -f https://raw.githubusercontent.com/kubernetes/dashboard/v2.7.0/aio/deploy/recommended.yaml   > /dev/null 2>&1

# Kurz warten
sleep 5

# Admin User erstellen
log "Creating Dashboard admin user..."
kubectl create serviceaccount admin-user -n kubernetes-dashboard 2>/dev/null || true
kubectl create clusterrolebinding admin-user --clusterrole=cluster-admin --serviceaccount=kubernetes-dashboard:admin-user 2>/dev/null || true

# NodePort Service
log "Creating NodePort service for Dashboard..."
cat <<EOF | kubectl apply -f - > /dev/null 2>&1
apiVersion: v1
kind: Service
metadata:
  name: kubernetes-dashboard-nodeport
  namespace: kubernetes-dashboard
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
DASHBOARD_TOKEN=$(kubectl -n kubernetes-dashboard create token admin-user 2>/dev/null || echo "Token generation failed")

if [ "$DASHBOARD_TOKEN" = "Token generation failed" ]; then
    log_warn "Could not generate Dashboard token automatically"
    log_warn "You can get it manually later by running:"
    log_warn "kubectl -n kubernetes-dashboard create token admin-user"
    DASHBOARD_TOKEN="<Token generation failed - see instructions above>"
else
    log "Dashboard token generated successfully"
fi

# 9. Service-Informationen sammeln und in .env speichern
log "Collecting service information..."

# Host IP ermitteln
HOST_IP=$(hostname -I | awk '{print $1}')

# NodePorts ermitteln
VAULT_NODE_PORT=30200
KAFKA_NODE_PORT=30092
PROMETHEUS_NODE_PORT=30090
GRAFANA_NODE_PORT=30000
DASHBOARD_NODE_PORT=30443

# Datei für Umgebungsvariablen erstellen
mkdir -p "$SCRIPT_DIR/tmp"
ENV_FILE="$SCRIPT_DIR/tmp/.env"
> "$ENV_FILE"

# Umgebungsvariablen speichern
save_env_var "KUBERNETES_DASHBOARD_URL" "https://$HOST_IP:$DASHBOARD_NODE_PORT"
save_env_var "KUBERNETES_DASHBOARD_TOKEN" "$DASHBOARD_TOKEN"

save_env_var "VAULT_ADDR" "http://$HOST_IP:$VAULT_NODE_PORT"
save_env_var "VAULT_TOKEN" "myroot"
save_env_var "KAFKA_BROKER" "$HOST_IP:$KAFKA_NODE_PORT"

save_env_var "POSTGRES_HOST" "postgres-postgresql.database.svc.cluster.local"
save_env_var "POSTGRES_PORT" "5432"
save_env_var "POSTGRES_USER" "dev"
save_env_var "POSTGRES_PASSWORD" "testPassword1234"
save_env_var "POSTGRES_DATABASE" "main_db"

save_env_var "REDIS_HOST" "redis-master.redis.svc.cluster.local"
save_env_var "REDIS_PORT" "6379"

save_env_var "PROMETHEUS_URL" "http://$HOST_IP:$PROMETHEUS_NODE_PORT"
save_env_var "GRAFANA_URL" "http://$HOST_IP:$GRAFANA_NODE_PORT"
save_env_var "GRAFANA_USER" "admin"
save_env_var "GRAFANA_PASSWORD" "admin"

save_env_var "CERT_MANAGER_NAMESPACE" "cert-manager"

log "Environment variables saved to $ENV_FILE"

# 10. Service-Informationen anzeigen
log "===================================="
log "Installation Complete!"
log "===================================="

echo ""
log "Access Information:"
echo "------------------------------"

# Kubernetes Dashboard Info
log "Kubernetes Dashboard:"
log "   URL: https://$HOST_IP:$DASHBOARD_NODE_PORT"
# Token nur loggen, wenn es erfolgreich generiert wurde
if [ "$DASHBOARD_TOKEN" != "<Token generation failed - see instructions above>" ]; then
    # Zeige nur die ersten und letzten Zeichen fuer Sicherheit, wenn es sehr lang ist
    if [ ${#DASHBOARD_TOKEN} -gt 50 ]; then
        log "   Token: ${DASHBOARD_TOKEN:0:10}...${DASHBOARD_TOKEN: -10}" # Zeige Anfang und Ende
        log "   (Full token saved to .env file)"
    else
        log "   Token: $DASHBOARD_TOKEN"
    fi
else
    log "   Token: $DASHBOARD_TOKEN"
fi
echo ""

# Vault Info
log "Vault:"
log "   UI: http://$HOST_IP:$VAULT_NODE_PORT"
log "   Token: myroot"
echo ""

# Kafka Info
log "Kafka:"
log "   Broker: $HOST_IP:$KAFKA_NODE_PORT"
echo ""

# PostgreSQL Info
log "PostgreSQL:"
log "   Host: postgres-postgresql.database.svc.cluster.local"
log "   Port: 5432"
log "   User: dev"
log "   Password: testPassword1234"
log "   Database: main_db"
echo ""

# Redis Info
log "Redis:"
log "   Host: redis-master.redis.svc.cluster.local"
log "   Port: 6379"
echo ""

# Monitoring Info
log "Monitoring:"
log "   Prometheus: http://$HOST_IP:$PROMETHEUS_NODE_PORT"
log "   Grafana: http://$HOST_IP:$GRAFANA_NODE_PORT"
log "   Grafana User: admin"
log "   Grafana Password: admin"
echo ""

# cert-manager Info
log "cert-manager:"
log "   Namespace: cert-manager"
log "   Status: Installed (waiting for configuration)"
echo ""

log "Your Ventra stack is ready!"
log "Use 'kubectl get pods -A' to see all running services"
log "Environment variables have been saved to $ENV_FILE"
log "Next step: Run 'configure-vault-cert-manager.sh' after setting VAULT_TOKEN"