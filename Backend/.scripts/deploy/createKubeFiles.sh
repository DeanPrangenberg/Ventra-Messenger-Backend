#!/bin/bash
set -euo pipefail

K8S_DIR="../../k8sFiles"
mkdir -p "$K8S_DIR"

SERVICES=(vm-api vm-auth vm-core vm-md vm-logger vm-redis-api)
NAMESPACE="default"

for SERVICE in "${SERVICES[@]}"; do
  SA_FILE="${K8S_DIR}/${SERVICE}-sa.yaml"
  CONFIGMAP_FILE="${K8S_DIR}/${SERVICE}-vault-agent-config.yaml"
  DEPLOY_FILE="${K8S_DIR}/${SERVICE}-deployment.yaml"

  # ServiceAccount YAML
  cat > "$SA_FILE" <<EOF
apiVersion: v1
kind: ServiceAccount
metadata:
  name: ${SERVICE}-sa
  namespace: ${NAMESPACE}
EOF

  # Vault Agent ConfigMap YAML
  cat > "$CONFIGMAP_FILE" <<EOF
apiVersion: v1
kind: ConfigMap
metadata:
  name: ${SERVICE}-vault-agent-config
  namespace: ${NAMESPACE}
data:
  config.hcl: |
    auto_auth {
      method "kubernetes" {
        mount_path = "auth/kubernetes"
        config = {
          role = "${SERVICE}"
        }
      }
      sink "file" {
        config = {
          path = "/etc/tls/vault-token"
        }
      }
    }
    template {
      destination = "/etc/tls/cert.pem"
      contents = <<EOH
    {{ with secret "pki-int/issue/${SERVICE}" "common_name=${SERVICE}.svc.cluster.local" }}
    {{ .Data.certificate }}
    {{ end }}
    EOH
    }
    template {
      destination = "/etc/tls/key.pem"
      contents = <<EOH
    {{ with secret "pki-int/issue/${SERVICE}" "common_name=${SERVICE}.svc.cluster.local" }}
    {{ .Data.private_key }}
    {{ end }}
    EOH
    }
    template {
      destination = "/etc/tls/ca.pem"
      contents = <<EOH
    {{ with secret "pki-int/issue/${SERVICE}" "common_name=${SERVICE}.svc.cluster.local" }}
    {{ .Data.issuing_ca }}
    {{ end }}
    EOH
    }
EOF

 if [ -f "$DEPLOY_FILE" ]; then
   # 1. Entferne alte volumeMounts (nur wenn volumeMounts existieren)
   # Prüfe, ob der Pfad überhaupt existiert, bevor du 'del' versuchst
   if yq -e '.spec.template.spec.containers[].volumeMounts' "$DEPLOY_FILE" > /dev/null 2>&1; then
     yq -i -y 'del(.spec.template.spec.containers[].volumeMounts[] | select(.mountPath == "/tmp/ventra/ca.crt" or .mountPath == "/tmp/ventra/client.crt" or .mountPath == "/tmp/ventra/client.key"))' "$DEPLOY_FILE"
   else
     echo "  Keine bestehenden volumeMounts zum Löschen in $DEPLOY_FILE gefunden."
   fi

   # 2. Füge emptyDir Volume hinzu (nur wenn es noch nicht existiert)
   # Prüfe, ob das Volume bereits existiert, um Duplikate zu vermeiden
   if ! yq -e '.spec.template.spec.volumes[] | select(.name == "vault-certs")' "$DEPLOY_FILE" > /dev/null 2>&1; then
     yq -i -y '.spec.template.spec.volumes += [{"name": "vault-certs", "emptyDir": {}}]' "$DEPLOY_FILE"
   else
     echo "  Volume 'vault-certs' existiert bereits in $DEPLOY_FILE."
   fi

   # 3. Füge volumeMount zum Haupt-Container hinzu (nur wenn er nicht existiert)
   # Prüfe auf den Mount im ersten Container
   if ! yq -e '.spec.template.spec.containers[0].volumeMounts[] | select(.name == "vault-certs")' "$DEPLOY_FILE" > /dev/null 2>&1; then
     yq -i -y '.spec.template.spec.containers[0].volumeMounts += [{"name": "vault-certs", "mountPath": "/etc/tls"}]' "$DEPLOY_FILE"
   else
     echo "  VolumeMount 'vault-certs' existiert bereits im Hauptcontainer von $DEPLOY_FILE."
   fi

   # 4. Füge Vault Agent Sidecar hinzu (nur wenn er nicht existiert)
   # Prüfe, ob ein Container mit dem Namen 'vault-agent' bereits existiert
   if ! yq -e '.spec.template.spec.containers[] | select(.name == "vault-agent")' "$DEPLOY_FILE" > /dev/null 2>&1; then
     yq -i -y '.spec.template.spec.containers += [{
       "name": "vault-agent",
       "image": "hashicorp/vault:1.15.0",
       "args": ["agent", "-config=/etc/vault/config/config.hcl"],
       "env": [{"name": "VAULT_ADDR", "value": "https://vm-vault:8200"}],
       "volumeMounts": [
         {"name": "vault-certs", "mountPath": "/etc/tls"},
         {"name": "vault-agent-config", "mountPath": "/etc/vault/config"}
       ]
     }]' "$DEPLOY_FILE"
   else
     echo "  Vault Agent Sidecar existiert bereits in $DEPLOY_FILE."
   fi

   # 5. Füge ConfigMap Volume hinzu (nur wenn es nicht existiert)
    # Prüfe, ob das Volume bereits existiert
   if ! yq -e '.spec.template.spec.volumes[] | select(.name == "vault-agent-config")' "$DEPLOY_FILE" > /dev/null 2>&1; then
      # Verwende eine Variable für den ConfigMap-Namen, um das Escaping zu vereinfachen
      CONFIGMAP_NAME="${SERVICE}-vault-agent-config"
      yq -i -y ".spec.template.spec.volumes += [{\"name\": \"vault-agent-config\", \"configMap\": {\"name\": \"${CONFIGMAP_NAME}\"}}]" "$DEPLOY_FILE"
   else
     echo "  Volume 'vault-agent-config' existiert bereits in $DEPLOY_FILE."
   fi

   # 6. Setze serviceAccountName (immer, da es überschrieben werden kann)
   # Dieser Befehl ist in der Regel sicher, da er einen Wert setzt oder überschreibt.
   yq -i -y ".spec.template.spec.serviceAccountName = \"${SERVICE}-sa\"" "$DEPLOY_FILE"

 fi

  echo "Created and patched $SA_FILE, $CONFIGMAP_FILE, $DEPLOY_FILE"
done

echo "Generated Kubernetes manifests for ServiceAccounts, ConfigMaps, and patched Deployments in $K8S_DIR"
# List of services and their PVC names/paths
declare -A PVC_PATHS=(
  [vm-vault-data]="/.vault/data"
  [vm-main-db-data]="/var/lib/postgresql/data"
  [vm-redis-data]="/data"
  [kafka-data]="/var/lib/kafka/data"
  [kafka-zookeeper-data]="/var/lib/zookeeper/data"
)

for pvc in "${!PVC_PATHS[@]}"; do
  cat > "${K8S_DIR}/${pvc}-pvc.yaml" <<EOF
apiVersion: v1
kind: PersistentVolumeClaim
metadata:
  name: ${pvc}
spec:
  accessModes:
    - ReadWriteOnce
  resources:
    requests:
      storage: 1Gi
EOF
  echo "Created PVC manifest: ${K8S_DIR}/${pvc}-pvc.yaml"
done
