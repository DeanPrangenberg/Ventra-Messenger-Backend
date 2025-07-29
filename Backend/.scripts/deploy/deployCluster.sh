#!/bin/bash
set -euo pipefail

cd "$(dirname "$0")/../.." || exit 1
K8S_DIR="k8sFiles"
DOCKERCOMPOSE_FILE="docker-compose.yml"

# --- NEU: Definition der Images, die gebaut und importiert werden müssen ---
# Format: <Image-Name>:<Pfad-zum-Dockerfile>
# Passe diese Pfade zu deinen tatsächlichen Dockerfile-Orten an.
declare -A IMAGE_BUILD_SPECS=(
  ["backend-vm-api"]="VM-API/Dockerfile"
  ["backend-vm-auth"]="VM-AUTH/Dockerfile"
  ["backend-vm-core"]="VM-CORE/Dockerfile"
  ["backend-vm-md"]="VM-MD/Dockerfile"
  ["backend-vm-logger"]="VM-LOGGER/Dockerfile"
  ["backend-vm-redis-api"]="VM-REDIS-API/Dockerfile"
  ["kafka-init"]="init-kafka/Dockerfile" # Annahme basierend auf docker-compose.yml
  ["vm-vault-init"]="init-vault/Dockerfile" # Annahme basierend auf docker-compose.yml
  # Füge hier weitere selbstgebaute Images hinzu, wenn nötig
)
# --- ENDE NEU ---

# --- NEU: Funktion zum Bauen und Importieren von Images ---
build_and_import_images() {
  echo "Building and importing local Docker images into k3s..."

  for image_name in "${!IMAGE_BUILD_SPECS[@]}"; do
    dockerfile_path="${IMAGE_BUILD_SPECS[$image_name]}"
    echo "  Building image: $image_name from $dockerfile_path"

    # Baue das Image mit Docker
    if ! docker build -t "$image_name" -f "$dockerfile_path" . ; then
       echo "ERROR: Failed to build image $image_name"
       exit 1
    fi

    # Exportiere das Image als tar
    tar_file="${image_name}.tar"
    echo "  Exporting image $image_name to $tar_file"
    if ! docker save "$image_name":latest > "$tar_file"; then
        echo "ERROR: Failed to export image $image_name"
        exit 1
    fi

    # Importiere das Image in k3s
    echo "  Importing $tar_file into k3s"
    if ! sudo k3s ctr images import "$tar_file"; then
        echo "ERROR: Failed to import image $image_name into k3s"
        # Optional: Bereinige die temporäre tar-Datei bei Fehler nicht
        # rm -f "$tar_file"
        exit 1
    fi

    # Lösche die temporäre tar-Datei
    echo "  Cleaning up $tar_file"
    rm -f "$tar_file"
  done
  echo "Finished building and importing images."
}
# --- ENDE NEU ---

# --- NEU: Robustere Bereinigungsfunktion ---
cleanup_cluster_resources() {
  echo "Attempting robust cleanup of previous Kubernetes resources..."

  # 1. Lösche Deployments, Pods, Services etc. aus dem Manifest-Verzeichnis
  if [ -d "$K8S_DIR" ] && [ -n "$(ls -A "$K8S_DIR")" ]; then
    echo "  Deleting resources defined in $K8S_DIR..."
    kubectl delete -f "$K8S_DIR" --wait=false 2>/dev/null || true
    # Warte einen Moment, damit das Löschen beginnen kann
    sleep 5
  fi

  # 2. Warte und erzwinge das Löschen von hängenden Pods (wichtig für ImagePullBackOff)
  echo "  Force deleting pods stuck in ImagePullBackOff/ErrImagePull..."
  kubectl delete pods --all --force --grace-period=0 --timeout=30s 2>/dev/null || true

  # 3. Lösche PVCs explizit (sehr wichtig für local-path StorageClass Probleme)
  echo "  Deleting PersistentVolumeClaims..."
  kubectl delete pvc --all --timeout=30s 2>/dev/null || true

  # 4. Warte, bis die PVCs und PVs tatsächlich gelöscht sind
  echo "  Waiting for PVCs and PVs to be fully deleted..."
  # Einfache Warte-Schleife (max 60 Sekunden)
  wait_counter=0
  while [ $wait_counter -lt 60 ] && [ "$(kubectl get pvc --no-headers 2>/dev/null | wc -l)" -gt 0 ]; do
    echo "    Still waiting for PVCs to be deleted..."
    sleep 2
    wait_counter=$((wait_counter + 1))
  done
  if [ $wait_counter -ge 60 ]; then
    echo "    WARNING: Timed out waiting for PVCs to delete. Some might still be terminating."
  else
    echo "    PVCs deleted."
  fi

  # 5. Entferne das alte Manifest-Verzeichnis
  if [ -d "$K8S_DIR" ]; then
    echo "  Removing old manifest directory $K8S_DIR..."
    rm -rf "$K8S_DIR"
  fi

  # 6. Erstelle das Verzeichnis neu
  mkdir "$K8S_DIR"
  echo "Cleanup finished."
}
# --- ENDE NEU ---

echo "=== Starting Kubernetes Deployment ==="

# --- AKTUALISIERT: Verwende die neue Bereinigungsfunktion ---
cleanup_cluster_resources
# --- AKTUALISIERT: Baue und importiere Images ---
build_and_import_images

echo "Converting docker-compose.yml to Kubernetes manifests..."
kompose convert -f "$DOCKERCOMPOSE_FILE" --out "$K8S_DIR"

echo "Patching Deployments/DaemonSets/StatefulSets to set imagePullPolicy for local images..."
# Liste der lokalen Images, die du importiert hast
LOCAL_IMAGES=("backend-vm-api" "backend-vm-auth" "backend-vm-core" "backend-vm-md" "backend-vm-logger" "backend-vm-redis-api" "kafka-init" "vm-vault-init") # Füge ggf. weitere hinzu

# Finde alle relevanten YAML-Dateien (Deployments, StatefulSets, DaemonSets)
find "$K8S_DIR" -type f -name '*.yaml' | while read -r file; do
  # Prüfe, ob die Datei eine dieser Arten ist
  if yq -e '.kind == "Deployment" or .kind == "StatefulSet" or .kind == "DaemonSet"' "$file" > /dev/null; then
    echo "  Processing $file..."
    # Iteriere über die Liste der lokalen Images
    for img_name in "${LOCAL_IMAGES[@]}"; do
      # Prüfe, ob das Image in der Datei verwendet wird (irgendwo unter .spec.template.spec.containers[])
      if yq -e --arg in "$img_name" '.spec.template.spec.containers[] | select(.image == $in)' "$file" > /dev/null; then
        echo "    Found container using image '$img_name'. Setting imagePullPolicy to IfNotPresent."
        # Setze imagePullPolicy für *alle* Container mit diesem Image-Namen in dieser Ressource
        # yq -i -y ist veraltet, benutze -i mit dem passenden Ausgabeformat
        yq -i -y "(.spec.template.spec.containers[] | select(.image == \"$img_name\")).imagePullPolicy = \"IfNotPresent\"" "$file"
      fi
    done
  fi
done
echo "Finished patching imagePullPolicy."

echo "Patching all PVCs to use accessMode: ReadWriteOnce and set storage requests..."
# --- AKTUALISIERT: Robusteres Patchen, um ReadWriteOncePod zu erlauben, falls kompose es generiert ---
find "$K8S_DIR" -type f -name '*.yaml' | while read -r file; do
  if yq -e '.kind == "PersistentVolumeClaim"' "$file" >/dev/null; then
    # Setze den ersten AccessMode auf ReadWriteOnce, falls er nicht bereits ReadWriteOncePod ist
    current_mode=$(yq -r '.spec.accessModes[0]' "$file")
    if [[ "$current_mode" != "ReadWriteOncePod" ]]; then
        yq -i -y '.spec.accessModes = ["ReadWriteOnce"]' "$file"
    fi
    # Setze Storage Request, falls nicht vorhanden
    yq -i -y '.spec.resources.requests.storage = (.spec.resources.requests.storage // "100Mi")' "$file"
  fi
done

echo "Validating all PVCs for correct accessModes and storage requests..."
err=0
# Use process substitution to avoid subshell issues with 'err' variable
while read -r file; do
  if yq -e '.kind == "PersistentVolumeClaim"' "$file" >/dev/null; then
    # Use yq -r to get the raw string value
    mode=$(yq -r '.spec.accessModes[0]' "$file")
    storage=$(yq -r '.spec.resources.requests.storage' "$file")
    # Erlaube sowohl ReadWriteOnce als auch ReadWriteOncePod
    if [[ "$mode" != "ReadWriteOnce" && "$mode" != "ReadWriteOncePod" ]]; then
      echo "ERROR: $file has unsupported accessMode $mode"
      err=1
    fi
    # Check if storage is empty or null (yq -r outputs 'null' if key is missing)
    if [[ -z "$storage" || "$storage" == "null" ]]; then
      echo "ERROR: $file missing storage request"
      err=1
    fi
  fi
done < <(find "$K8S_DIR" -type f -name '*.yaml') # Process substitution

if [[ $err -ne 0 ]]; then
  echo "PVC validation failed. Fix errors above."
  exit 1
fi

echo "Generating additional Kubernetes files (ServiceAccounts, ConfigMaps)..."
./.scripts/deploy/createKubeFiles.sh

echo "Applying manifests to the k3s cluster..."
kubectl apply -f "$K8S_DIR"

echo "Showing cluster status:"
kubectl get all

echo "Adding Dashboard..."
kubectl apply -f https://raw.githubusercontent.com/kubernetes/dashboard/v2.7.0/aio/deploy/recommended.yaml

echo "Creating admin-user and binding (if not present)..."
kubectl create serviceaccount admin-user -n kubernetes-dashboard 2>/dev/null || true
kubectl create clusterrolebinding admin-user --clusterrole=cluster-admin --serviceaccount=kubernetes-dashboard:admin-user 2>/dev/null || true

echo "Getting Dashboard login token..."
TOKEN=$(kubectl -n kubernetes-dashboard create token admin-user)

DASHBOARD_URL="http://localhost:8001/api/v1/namespaces/kubernetes-dashboard/services/https:kubernetes-dashboard:/proxy/"

echo "Starting proxy for Dashboard (runs in background)..."
if ! lsof -i:8001 >/dev/null 2>&1; then
  kubectl proxy &
  # Gib dem Proxy einen Moment Zeit zu starten
  sleep 2
else
  echo "Port 8001 already in use, not starting another proxy."
fi

echo
echo "=============================================="
echo "Deployment process completed."
echo "Cluster status:"
kubectl get all
echo
echo "Kubernetes Dashboard is available at:"
echo "$DASHBOARD_URL"
echo
echo "Login Token (copy this for dashboard login):"
echo "$TOKEN"
echo "=============================================="
echo "Next steps:"
echo "1. Check pod status: kubectl get pods"
echo "2. Check pod logs for errors: kubectl logs <pod-name>"
echo "3. Check events: kubectl get events --sort-by=.metadata.creationTimestamp -A"
echo "=============================================="