#!/bin/bash
# configure-vault-cert-manager.sh - Konfiguriert Vault für cert-manager Integration
# ==================== KONFIGURATION ====================
# cert-manager Version (empfohlen: aktuelle stabile Version)
CERT_MANAGER_VERSION="${CERT_MANAGER_VERSION:-v1.14.4}"
# Namespace für cert-manager (aus tmp/.env laden)
CERT_MANAGER_NAMESPACE="${CERT_MANAGER_NAMESPACE:-cert-manager}"
# Vault Einstellungen (aus tmp/.env laden)
VAULT_TOKEN="${VAULT_TOKEN:-}" # Muss gesetzt werden
# Vault AppRole für cert-manager (wird erstellt)
CERT_MANAGER_ROLE_NAME="${CERT_MANAGER_ROLE_NAME:-cert-manager}"
CERT_MANAGER_POLICY_NAME="${CERT_MANAGER_POLICY_NAME:-cert-manager-pki}"
# Vault PKI Pfade (Standardwerte, anpassen wenn nötig)
ROOT_PKI_PATH="${ROOT_PKI_PATH:-pki}"
INTERMEDIATE_PKI_PATH="${INTERMEDIATE_PKI_PATH:-pki_int}"
# Service-Zertifikat Rolle (Standardwert, anpassen wenn nötig)
SERVICE_CERT_ROLE="${SERVICE_CERT_ROLE:-internal-services}"
# Logging und Error Handling
LOG_LEVEL="${LOG_LEVEL:-INFO}"
# ==================== ENDE KONFIGURATION ====================
# ==================== HILFSFUNKTIONEN ====================
log() {
    local level="$1"
    local message="$2"
    local timestamp=$(date -u '+%Y-%m-%dT%H:%M:%SZ')
    echo "[$timestamp] [$level] $message"
}
info() {
    if [[ "$LOG_LEVEL" == "INFO" ]] || [[ "$LOG_LEVEL" == "DEBUG" ]]; then
        log "INFO" "$1"
    fi
}
warn() {
    log "WARN" "$1"
}
error() {
    log "ERROR" "$1"
}
handle_error() {
    local line_number="$1"
    local error_code="$2"
    error "Error at line $line_number with exit code $error_code"
    cleanup_temp_files
    exit "$error_code"
}
trap 'handle_error $LINENO $?' ERR
set -eE

# Neue Funktion zum Speichern von Umgebungsvariablen für cert-manager Konfiguration
save_cm_env_var() {
    local var_name="$1"
    local var_value="$2"

    # Escape potenziell problematische Zeichen in Werten
    escaped_value=$(printf '%s\n' "$var_value" | sed 's/[$`\\]/\\&/g')

    echo "$var_name=$escaped_value" >> "$CM_ENV_FILE"
}
# ==================== INSTALLATIONS- UND KONFIGURATIONSFUNKTIONEN ====================
check_prerequisites() {
    info "Prüfe Voraussetzungen..."
    if ! command -v kubectl &> /dev/null; then
        error "kubectl ist nicht installiert"
        exit 1
    fi
    if ! command -v vault &> /dev/null; then
        error "vault CLI ist nicht installiert"
        exit 1
    fi
    # Prüfe Kubernetes Cluster Verbindung
    if ! kubectl cluster-info &> /dev/null; then
        error "Keine Verbindung zum Kubernetes Cluster"
        exit 1
    fi
    # Prüfe Vault Verbindung
    if [[ -z "$VAULT_TOKEN" ]]; then
        error "VAULT_TOKEN muss gesetzt werden (z.B. export VAULT_TOKEN='s.token')"
        exit 1
    fi
    export VAULT_ADDR="$VAULT_ADDR"
    export VAULT_TOKEN="$VAULT_TOKEN"
    if ! vault status &> /dev/null; then
        error "Keine Verbindung zu Vault unter $VAULT_ADDR"
        exit 1
    fi
    info "Alle Voraussetzungen erfüllt"
}
# Funktionen install_cert_manager und create_kubernetes_secret entfernt

create_vault_policy() {
    info "Erstelle Vault Policy für cert-manager..."
    local policy_hcl=$(cat <<EOF
# Erlaubt das Lesen der CA und CRL vom Intermediate PKI
path "${INTERMEDIATE_PKI_PATH}/ca" {
  capabilities = ["read"]
}
path "${INTERMEDIATE_PKI_PATH}/crl" {
  capabilities = ["read"]
}
# Erlaubt das Ausstellen von Zertifikaten über die konfigurierte Rolle
path "${INTERMEDIATE_PKI_PATH}/issue/${SERVICE_CERT_ROLE}" {
  capabilities = ["create", "update"]
}
# Optional: Erlaubt das Lesen der Rolle (für Informationen)
path "${INTERMEDIATE_PKI_PATH}/roles/${SERVICE_CERT_ROLE}" {
  capabilities = ["read"]
}
# Für Zertifikatswiderruf (optional)
# path "${INTERMEDIATE_PKI_PATH}/revoke" {
#   capabilities = ["create", "update"]
# }
# Für Statusprüfung (optional)
# path "sys/health" {
#   capabilities = ["read"]
# }
EOF
)
    echo "$policy_hcl" | vault policy write "$CERT_MANAGER_POLICY_NAME" -
    info "Vault Policy '$CERT_MANAGER_POLICY_NAME' erstellt"
}
create_vault_approle() {
    info "Erstelle Vault AppRole für cert-manager..."
    # AppRole erstellen
    vault write auth/approle/role/"$CERT_MANAGER_ROLE_NAME" \
        token_policies="$CERT_MANAGER_POLICY_NAME" \
        token_ttl="20m" \
        token_max_ttl="1h" \
        secret_id_ttl="0" \
        token_bound_cidrs="" \
        secret_id_bound_cidrs="" \
        token_num_uses=0
    # RoleID abrufen
    local role_id
    role_id=$(vault read -field=role_id auth/approle/role/"$CERT_MANAGER_ROLE_NAME"/role-id)
    # SecretID erstellen (einmalig)
    local secret_id
    secret_id=$(vault write -f -field=secret_id auth/approle/role/"$CERT_MANAGER_ROLE_NAME"/secret-id)

    # Werte in Umgebungsvariablen speichern (für späteren Gebrauch innerhalb des Skripts)
    export CERT_MANAGER_VAULT_ROLE_ID="$role_id"
    export CERT_MANAGER_VAULT_SECRET_ID="$secret_id"

    # Werte auch in die tmp/.env.cm Datei speichern
    save_cm_env_var "CERT_MANAGER_VAULT_ROLE_ID" "$CERT_MANAGER_VAULT_ROLE_ID"
    save_cm_env_var "CERT_MANAGER_VAULT_SECRET_ID" "$CERT_MANAGER_VAULT_SECRET_ID"
    save_cm_env_var "CERT_MANAGER_ROLE_NAME" "$CERT_MANAGER_ROLE_NAME"
    save_cm_env_var "CERT_MANAGER_POLICY_NAME" "$CERT_MANAGER_POLICY_NAME"
    save_cm_env_var "ROOT_PKI_PATH" "$ROOT_PKI_PATH"
    save_cm_env_var "INTERMEDIATE_PKI_PATH" "$INTERMEDIATE_PKI_PATH"
    save_cm_env_var "SERVICE_CERT_ROLE" "$SERVICE_CERT_ROLE"


    info "Vault AppRole '$CERT_MANAGER_ROLE_NAME' erstellt"
    info "Credentials gespeichert in $CM_ENV_FILE"
    echo "Role ID: $role_id"
    # Secret ID wird nicht geloggt, da sie sensibel ist
}
create_kubernetes_secret() {
    info "Erstelle Kubernetes Secret mit Vault AppRole Credentials..."
    # Prüfen, ob die notwendigen Variablen gesetzt sind
    if [[ -z "$CERT_MANAGER_VAULT_ROLE_ID" ]] || [[ -z "$CERT_MANAGER_VAULT_SECRET_ID" ]]; then
        error "Vault AppRole Credentials nicht korrekt geladen. Stelle sicher, dass create_vault_approle zuerst ausgeführt wurde."
        exit 1
    fi
    # Kubernetes Secret erstellen
    kubectl create secret generic vault-approle \
        --namespace="$CERT_MANAGER_NAMESPACE" \
        --from-literal=role_id="$CERT_MANAGER_VAULT_ROLE_ID" \
        --from-literal=secret_id="$CERT_MANAGER_VAULT_SECRET_ID" \
        --dry-run=client -o yaml | kubectl apply -f -
    info "Kubernetes Secret 'vault-approle' im Namespace '$CERT_MANAGER_NAMESPACE' erstellt"
}
create_vault_issuer() {
    info "Erstelle VaultIssuer für cert-manager..."
    # Root CA Zertifikat von Vault holen (für caBundle)
    vault read -field=certificate "$ROOT_PKI_PATH"/ca > /tmp/vault-ca.crt
    # Base64-kodiertes CA-Bundle für den Issuer
    local ca_bundle
    ca_bundle=$(base64 -w 0 /tmp/vault-ca.crt)
    # Vault-Adresse für den Cluster (aus Konfiguration oder Umgebung)
    local cluster_vault_addr

    # Issuer YAML erstellen
    cat <<EOF | kubectl apply -f -
apiVersion: cert-manager.io/v1
kind: Issuer
metadata:
  name: vault-issuer
  namespace: $CERT_MANAGER_NAMESPACE
spec:
  vault:
    server: $cluster_vault_addr
    path: $INTERMEDIATE_PKI_PATH
    caBundle: $ca_bundle
    auth:
      appRole:
        path: approle
        roleId: "$CERT_MANAGER_VAULT_ROLE_ID"
        secretRef:
          name: vault-approle
          key: secret_id
EOF
    info "VaultIssuer 'vault-issuer' im Namespace '$CERT_MANAGER_NAMESPACE' erstellt"
    info "Vault Server URL für Cluster: $cluster_vault_addr"

    # Vault Issuer Informationen speichern
    save_cm_env_var "VAULT_ISSUER_NAME" "vault-issuer"
    save_cm_env_var "CLUSTER_VAULT_ADDR" "$cluster_vault_addr"
}
verify_installation() {
    info "Verifiziere cert-manager Konfiguration..."
    # Warte etwas auf den Webhook
    sleep 10
    # Prüfe Issuer Status
    info "Prüfe VaultIssuer Status..."
    kubectl get issuer vault-issuer -n "$CERT_MANAGER_NAMESPACE" || warn "Issuer nicht gefunden oder nicht bereit"
    # Versuche einen Test CertificateRequest (optional)
    info "Erstelle Testzertifikat..."
    cat <<EOF | kubectl apply -f -
apiVersion: cert-manager.io/v1
kind: Certificate
metadata:
  name: test-cert
  namespace: $CERT_MANAGER_NAMESPACE
spec:
  secretName: test-cert-tls
  issuerRef:
    name: vault-issuer
    kind: Issuer
  commonName: test.cert-manager.svc.cluster.local
  dnsNames:
  - test.cert-manager.svc.cluster.local
EOF
    # Warte auf Zertifikat (max 60 Sekunden)
    info "Warte auf Testzertifikat-Ausstellung..."
    for i in {1..12}; do
        if kubectl get certificate test-cert -n "$CERT_MANAGER_NAMESPACE" -o jsonpath='{.status.conditions[?(@.type=="Ready")].status}' 2>/dev/null | grep -q "True"; then
            info "✅ Testzertifikat erfolgreich ausgestellt!"
            kubectl delete certificate test-cert -n "$CERT_MANAGER_NAMESPACE" > /dev/null 2>&1 || true
            return 0
        fi
        info "Warte... ($i/12)"
        sleep 5
    done
    warn "⚠️  Testzertifikat wurde nicht innerhalb von 60 Sekunden ausgestellt"
    warn "Prüfe cert-manager Logs: kubectl logs -n $CERT_MANAGER_NAMESPACE deployment/cert-manager"
    warn "Prüfe Issuer Status: kubectl describe issuer vault-issuer -n $CERT_MANAGER_NAMESPACE"
}
cleanup_temp_files() {
    info "Räume temporäre Dateien auf..."
    rm -f /tmp/vault-ca.crt # /tmp/cert-manager-vault-credentials.sh wird nicht mehr verwendet
    info "Temporäre Dateien entfernt"
}
# ==================== HAUPTSKRIPT ====================
info "Starte cert-manager Konfiguration für Vault Integration..."
info "=================================================="

# Lade Umgebungsvariablen aus tmp/.env, falls vorhanden
if [[ -f "tmp/.env" ]]; then
    info "Lade Umgebungsvariablen aus tmp/.env"
    source tmp/.env
else
    warn "tmp/.env Datei nicht gefunden. Stelle sicher, dass setup-ventra-stack-fast.sh zuerst ausgeführt wurde."
fi

# Datei für cert-manager spezifische Umgebungsvariablen erstellen
CM_ENV_FILE="tmp/.env.cm"
> "$CM_ENV_FILE" # Leere Datei erstellen

check_prerequisites
# install_cert_manager entfernt
create_vault_policy
create_vault_approle
create_kubernetes_secret
create_vault_issuer
verify_installation
cleanup_temp_files
info "=================================================="
info "cert-manager Konfiguration für Vault Integration ABGESCHLOSSEN!"
info "=================================================="
echo ""
info "ERSTELLTE RESOURCEN:"
info "  📦 Namespace: $CERT_MANAGER_NAMESPACE"
info "  🔐 Vault Policy: $CERT_MANAGER_POLICY_NAME"
info "  🔑 Vault AppRole: $CERT_MANAGER_ROLE_NAME"
info "  🤖 Kubernetes Secret: vault-approle (im cert-manager Namespace)"
info "  📜 cert-manager Issuer: vault-issuer (im cert-manager Namespace)"
echo ""
info "WICHTIGE INFORMATIONEN:"
info "  1. Vault AppRole Credentials sind in $CM_ENV_FILE gespeichert"
info "  2. Die Vault-Adresse für den Kubernetes-Cluster wurde verwendet: $CLUSTER_VAULT_ADDR"
info "  3. Der erstellte Issuer heißt 'vault-issuer' im Namespace '$CERT_MANAGER_NAMESPACE'"
echo ""
info "VERWENDUNG:"
info "  Um diese Konfiguration in anderen Skripten zu verwenden:"
info "    source tmp/.env"
info "    source tmp/.env.cm"
info "    # Dann können die Variablen wie \$VAULT_TOKEN, \$CERT_MANAGER_VAULT_ROLE_ID etc. verwendet werden"
echo ""
info "TROUBLESHOOTING:"
info "  Logs anzeigen: kubectl logs -n $CERT_MANAGER_NAMESPACE deployment/cert-manager"
info "  Issuer prüfen: kubectl describe issuer vault-issuer -n $CERT_MANAGER_NAMESPACE"
info "  Test erneut: kubectl apply -f- <<EOF ..."
info "apiVersion: cert-manager.io/v1"
info "kind: Certificate"
info "metadata:"
info "  name: debug-cert"
info "  namespace: $CERT_MANAGER_NAMESPACE"
info "spec:"
info "  secretName: debug-cert-tls"
info "  issuerRef:"
info "    name: vault-issuer"
info "    kind: Issuer"
info "  commonName: debug.cert-manager.svc.cluster.local"
info "EOF"
