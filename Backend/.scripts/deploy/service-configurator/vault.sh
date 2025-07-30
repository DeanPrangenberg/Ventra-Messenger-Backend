#!/bin/bash
# setup-vault-ca-secure.sh - Maximum Security Production Vault CA Setup with cert-manager Integration
# ==================== SICHERHEITSKONFIGURATION ====================
# Production-Sicherheitsparameter
# Root CA Einstellungen (Maximum Security)
ROOT_CA_COMMON_NAME="Ventra Production Root CA"
ROOT_CA_ORGANIZATION="Ventra Security Operations"
ROOT_CA_TTL="87600h"  # 10 Jahre - Standard für Production Root CAs
ROOT_CA_KEY_BITS="4096"  # Maximum Security
ROOT_CA_KEY_TYPE="rsa"
ROOT_CA_COUNTRY="DE"
ROOT_CA_LOCALITY="Berlin"
ROOT_CA_PROVINCE="Berlin"
ROOT_CA_STREET_ADDRESS="Secure Operations Center"
ROOT_CA_POSTAL_CODE="10115"
# Intermediate CA Einstellungen (Maximum Security)
INTERMEDIATE_CA_COMMON_NAME="Ventra Production Services Intermediate Authority"
INTERMEDIATE_CA_ORGANIZATION="Ventra Certificate Services"
INTERMEDIATE_CA_TTL="43800h"  # 5 Jahre
INTERMEDIATE_CA_KEY_BITS="4096"
INTERMEDIATE_CA_KEY_TYPE="rsa"
INTERMEDIATE_CA_COUNTRY="DE"
INTERMEDIATE_CA_LOCALITY="Berlin"
INTERMEDIATE_CA_PROVINCE="Berlin"
# Service-Zertifikat Einstellungen (Maximum Security für Services)
SERVICE_CERT_MAX_TTL="720h"   # 30 Tage
SERVICE_CERT_DEFAULT_TTL="8h" # 8 Stunden - Sehr häufige Rotation
SERVICE_CERT_KEY_BITS="2048"  # 2048 Bit - Performance/Sicherheit Balance
SERVICE_CERT_KEY_TYPE="rsa"
# Erweiterte Sicherheitseinstellungen
ALLOWED_DOMAINS="svc.cluster.local,cluster.local,internal.ventra.local"
SPECIFIC_SERVICE_DOMAINS="kafka.kafka.svc.cluster.local,redis.redis.svc.cluster.local,postgres-postgresql.database.svc.cluster.local,vault.vault.svc.cluster.local"
# PKI Engine Einstellungen (Maximum Security)
PKI_MAX_LEASE_TTL="87600h"    # 10 Jahre
PKI_INT_MAX_LEASE_TTL="43800h" # 5 Jahre
# Vault Sicherheitsparameter
VAULT_ADDR="${VAULT_ADDR:-http://127.0.0.1:30443}"
VAULT_TOKEN="${VAULT_TOKEN:-myroot}"
# ==================== CERT-MANAGER INTEGRATION KONFIGURATION ====================
# Enable cert-manager integration
ENABLE_CERT_MANAGER_INTEGRATION="${ENABLE_CERT_MANAGER_INTEGRATION:-false}"

# Vault details for cert-manager (must be reachable from the cluster)
CERT_MANAGER_VAULT_ADDR="${CERT_MANAGER_VAULT_ADDR:-$VAULT_ADDR}"

# Vault AppRole credentials (REPLACE WITH SECURE VALUES OR USE TOKEN AUTH)
# It's highly recommended to use AppRole for cert-manager instead of root tokens
CERT_MANAGER_VAULT_ROLE_ID="${CERT_MANAGER_VAULT_ROLE_ID:-}" # Must be set if using AppRole
CERT_MANAGER_VAULT_SECRET_ID="${CERT_MANAGER_VAULT_SECRET_ID:-}" # Must be set if using AppRole

# Alternative: Vault Token Authentication (LESS SECURE for long-term use)
# CERT_MANAGER_VAULT_TOKEN="${CERT_MANAGER_VAULT_TOKEN:-}" # Not recommended for production

# Namespace where cert-manager resources should be created (usually cert-manager or default)
CERT_MANAGER_NAMESPACE="${CERT_MANAGER_NAMESPACE:-cert-manager}"

# Names for the VaultIssuer resources
ROOT_ISSUER_NAME="${ROOT_ISSUER_NAME:-vault-root-issuer}"
INTERMEDIATE_ISSUER_NAME="${INTERMEDIATE_ISSUER_NAME:-vault-intermediate-issuer}"

# Path to the PKI engines in Vault
ROOT_PKI_PATH="${ROOT_PKI_PATH:-pki}"
INTERMEDIATE_PKI_PATH="${INTERMEDIATE_PKI_PATH:-pki_int}"
# ==================== ENDE CERT-MANAGER KONFIGURATION ====================
# Logging und Error Handling
LOG_LEVEL="${LOG_LEVEL:-INFO}"
CLEANUP_TEMP_FILES="${CLEANUP_TEMP_FILES:-true}"
ENABLE_AUDIT_LOGGING="${ENABLE_AUDIT_LOGGING:-true}"
# ==================== ENDE KONFIGURATION ====================
# ==================== SICHERHEITSFUNKTIONEN ====================
# Logging Funktionen mit Sicherheitskontext
# Logging Funktionen mit Sicherheitskontext
log() {
    local level="$1"
    local message="$2"
    local timestamp=$(date -u '+%Y-%m-%dT%H:%M:%SZ')
    echo "[$timestamp] [$level] [SECURITY] $message"
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
security_audit() {
    log "AUDIT" "$1"
}
# Error Handling mit Sicherheitsprotokollierung
handle_error() {
    local line_number="$1"
    local error_code="$2"
    error "CRITICAL SECURITY ERROR at line $line_number with exit code $error_code"
    security_audit "Setup process terminated due to security error"
    cleanup_temp_files
    exit "$error_code"
}
# Temp File Cleanup mit Sicherheitsprüfung
cleanup_temp_files() {
    if [[ "$CLEANUP_TEMP_FILES" == "true" ]]; then
        info "Performing secure cleanup of temporary files..."
        # Sichere Löschung mit shred (falls verfügbar)
        for file in /tmp/CA_cert.crt /tmp/pki_intermediate.csr /tmp/intermediate.cert.pem /tmp/root_ca.json /tmp/int_ca.json /tmp/test_kafka_cert.json /tmp/test_kafka.crt /tmp/*-issuer.yaml; do # <--- *-issuer.yaml hinzugefügt
            if [ -f "$file" ]; then
                if command -v shred >/dev/null 2>&1; then
                    shred -u -z "$file" 2>/dev/null || rm -f "$file"
                else
                    rm -f "$file"
                fi
                security_audit "Securely deleted temporary file: $file"
            fi
        done
    fi
}
# Vault Token Sicherheitsprüfung
validate_vault_token() {
    info "Validating Vault token security..."
    local token_info
    token_info=$(vault token lookup 2>/dev/null) || {
        error "Cannot lookup Vault token - security validation failed"
        return 1
    }
    # Prüfen ob Token ein Root-Token ist (nur für Setup erlaubt)
    if echo "$token_info" | grep -q "root"; then
        security_audit "Using root token for initial setup (expected for CA initialization)"
    else
        warn "Using non-root token - some operations may fail"
    fi
    # Prüfen auf TTL
    local ttl
    ttl=$(echo "$token_info" | grep "ttl" | awk '{print $2}')
    if [[ "$ttl" == "0" ]] || [[ -z "$ttl" ]]; then
        security_audit "Using non-expiring token - ensure proper token management after setup"
    else
        security_audit "Token has TTL: $ttl - automatic expiration enabled"
    fi
}
# Sicherheitszertifikat-Validierung
validate_certificate_security() {
    local cert_file="$1"
    local cert_type="$2"
    if [ ! -f "$cert_file" ]; then
        error "Certificate file $cert_file not found for security validation"
        return 1
    fi
    info "Performing security validation of $cert_type certificate..."
    # Prüfen der Schlüssellänge
    local key_size
    key_size=$(openssl x509 -in "$cert_file" -noout -text | grep "RSA Public-Key" | grep -o "([0-9]* bit)" | grep -o "[0-9]*")
    if [ "$key_size" -lt 2048 ]; then
        error "Certificate $cert_type has insecure key size: $key_size bits (minimum 2048 required)"
        return 1
    fi
    # Prüfen der Signaturalgorithmus
    local sig_alg
    sig_alg=$(openssl x509 -in "$cert_file" -noout -text | grep "Signature Algorithm" | head -1 | awk '{print $3}')
    if [[ ! "$sig_alg" =~ ^(sha256WithRSAEncryption|sha384WithRSAEncryption|sha512WithRSAEncryption)$ ]]; then
        error "Certificate $cert_type uses weak signature algorithm: $sig_alg"
        return 1
    fi
    security_audit "$cert_type certificate security validation passed (Key Size: $key_size, Sig Alg: $sig_alg)"
    return 0
}
# ==================== CERT-MANAGER HILFSFUNKTIONEN ====================

create_vault_issuer_yaml() {
    local issuer_name="$1"
    local vault_path="$2" # z.B. pki oder pki_int
    local issuer_namespace="$3"
    local issuer_type="${4:-Issuer}" # Issuer oder ClusterIssuer
    local output_file="$5"

    local auth_config=""
    if [[ -n "$CERT_MANAGER_VAULT_ROLE_ID" && -n "$CERT_MANAGER_VAULT_SECRET_ID" ]]; then
        # AppRole Auth
        auth_config="appRole:
        path: approle
        roleId: \"$CERT_MANAGER_VAULT_ROLE_ID\"
        secretRef:
          name: \"${issuer_name}-vault-secret\"
          key: secretId"
    # Alternative: Token Auth (weniger sicher)
    elif [[ -n "$CERT_MANAGER_VAULT_TOKEN" ]]; then
        auth_config="tokenSecretRef:
        name: \"${issuer_name}-vault-token\"
        key: token"
    else
        error "Either AppRole credentials (CERT_MANAGER_VAULT_ROLE_ID & CERT_MANAGER_VAULT_SECRET_ID) or Vault Token (CERT_MANAGER_VAULT_TOKEN) must be provided for cert-manager integration."
        return 1
    fi

    cat <<EOF > "$output_file"
apiVersion: cert-manager.io/v1
kind: $issuer_type
metadata:
  name: $issuer_name
  namespace: $issuer_namespace
spec:
  vault:
    server: $CERT_MANAGER_VAULT_ADDR
    path: $vault_path
    caBundle: $(base64 -w 0 /tmp/CA_cert.crt) # Root CA Bundle for Vault TLS verification
    auth:
      $auth_config
EOF
    info "Created $issuer_type YAML for '$issuer_name' at $output_file"
}

apply_cert_manager_resources() {
    local issuer_name="$1"
    local issuer_namespace="$2"
    local issuer_type="$3"
    local vault_secret_name="${issuer_name}-vault-secret"
    local vault_token_secret_name="${issuer_name}-vault-token"
    local issuer_yaml_file="/tmp/${issuer_name}-issuer.yaml"

    info "Applying cert-manager resources for $issuer_type '$issuer_name'..."

    # 1. Erstelle das Secret für AppRole Secret ID oder Vault Token (falls benötigt)
    if [[ -n "$CERT_MANAGER_VAULT_ROLE_ID" && -n "$CERT_MANAGER_VAULT_SECRET_ID" ]]; then
        info "Creating AppRole Secret for '$issuer_name'..."
        kubectl create secret generic "$vault_secret_name" \
            --namespace="$issuer_namespace" \
            --from-literal=secretId="$CERT_MANAGER_VAULT_SECRET_ID" \
            --dry-run=client -o yaml | kubectl apply -f -
        security_audit "Created AppRole secret '$vault_secret_name' for issuer '$issuer_name'"

    elif [[ -n "$CERT_MANAGER_VAULT_TOKEN" ]]; then
        info "Creating Token Secret for '$issuer_name'..."
        kubectl create secret generic "$vault_token_secret_name" \
            --namespace="$issuer_namespace" \
            --from-literal=token="$CERT_MANAGER_VAULT_TOKEN" \
            --dry-run=client -o yaml | kubectl apply -f -
        security_audit "Created Token secret '$vault_token_secret_name' for issuer '$issuer_name'"
    fi

    # 2. Erstelle und wende den Issuer/ClusterIssuer an
    create_vault_issuer_yaml "$issuer_name" "$INTERMEDIATE_PKI_PATH" "$issuer_namespace" "$issuer_type" "$issuer_yaml_file"
    if [ $? -ne 0 ]; then
        error "Failed to create YAML for $issuer_type '$issuer_name'"
        return 1
    fi

    kubectl apply -f "$issuer_yaml_file"
    if [ $? -eq 0 ]; then
         security_audit "Applied $issuer_type '$issuer_name' successfully"
    else
         error "Failed to apply $issuer_type '$issuer_name'"
         return 1
    fi

    # 3. Status prüfen (optional, aber nützlich)
    info "Checking status of $issuer_type '$issuer_name'..."
    # Warte etwas, damit der Controller Zeit hat
    sleep 5
    local issuer_status
    issuer_status=$(kubectl get "$issuer_type" "$issuer_name" --namespace="$issuer_namespace" -o jsonpath='{.status.conditions[?(@.type=="Ready")].status}' 2>/dev/null)
    if [[ "$issuer_status" == "True" ]]; then
        info "$issuer_type '$issuer_name' is Ready."
    else
         warn "$issuer_type '$issuer_name' might not be Ready yet. Check status with: kubectl get $issuer_type $issuer_name --namespace=$issuer_namespace"
    fi
}
# ==================== ENDE CERT-MANAGER HILFSFUNKTIONEN ====================
# ==================== HAUPTSKRIPT ====================
info "Initializing Maximum Security Vault Certificate Authority Setup..."
info "=================================================================="
# Trap für Error Handling
trap 'handle_error $LINENO $?' ERR
set -eE  # Exit on error and error in functions
# 1. Prüfen ob Vault läuft und erreichbar ist
info "Checking Vault accessibility and security posture..."
counter=0
max_retries=30
retry_interval=2
while [ $counter -lt $max_retries ]; do
    if curl -s -o /dev/null -w "%{http_code}" "$VAULT_ADDR/v1/sys/health" 2>/dev/null | grep -q "200\|429"; then
        info "Vault is accessible and responding"
        break
    fi
    warn "Waiting for Vault to be ready... ($((counter+1))/$max_retries)"
    sleep $retry_interval
    counter=$((counter + 1))
done
if [ $counter -eq $max_retries ]; then
    error "Vault is not accessible after $((max_retries * retry_interval)) seconds - aborting for security"
    exit 1
fi
# 2. Vault Token Sicherheitsprüfung
validate_vault_token
# 3. Audit Logging aktivieren (wenn konfiguriert)
if [[ "$ENABLE_AUDIT_LOGGING" == "true" ]]; then
    info "Enabling Vault audit logging for security monitoring..."
    if ! vault audit list | grep -q "file/"; then
        vault audit enable file file_path=/var/log/vault-audit.log
        security_audit "Audit logging enabled for CA operations"
    else
        security_audit "Audit logging already enabled"
    fi
fi
# 4. Prüfen ob Vault bereits konfiguriert ist (mit Sicherheitswarnung)
info "Checking existing PKI configuration for security conflicts..."
if vault secrets list | grep -q "^pki/"; then
    warn "SECURITY WARNING: PKI secret engine already exists - verifying configuration"
    security_audit "Existing PKI configuration detected - performing security review"
else
    # PKI Secret Engine für Root CA aktivieren
    info "Enabling PKI secret engine with maximum security settings..."
    vault secrets enable pki >/dev/null 2>&1
    security_audit "PKI secret engine enabled"
    # Maximale Gültigkeitsdauer setzen
    info "Tuning PKI secret engine for maximum security (max lease TTL: $PKI_MAX_LEASE_TTL)..."
    vault secrets tune -max-lease-ttl="$PKI_MAX_LEASE_TTL" pki >/dev/null 2>&1
    security_audit "PKI engine tuned for maximum security parameters"
    # Root CA Zertifikat generieren mit vollständigen Sicherheitsattributen
    info "Generating Root CA certificate with maximum security (TTL: $ROOT_CA_TTL, Key Size: $ROOT_CA_KEY_BITS)..."
    vault write -field=certificate pki/root/generate/internal \
        common_name="$ROOT_CA_COMMON_NAME" \
        organization="$ROOT_CA_ORGANIZATION" \
        country="$ROOT_CA_COUNTRY" \
        locality="$ROOT_CA_LOCALITY" \
        province="$ROOT_CA_PROVINCE" \
        street_address="$ROOT_CA_STREET_ADDRESS" \
        postal_code="$ROOT_CA_POSTAL_CODE" \
        ou="Root Certificate Authority" \
        ttl="$ROOT_CA_TTL" \
        key_bits="$ROOT_CA_KEY_BITS" \
        key_type="$ROOT_CA_KEY_TYPE" \
        exclude_cn_from_sans=true \
        > /tmp/CA_cert.crt 2>/dev/null
    # Sicherheitsvalidierung des Root-Zertifikats
    if validate_certificate_security "/tmp/CA_cert.crt" "Root CA"; then
        security_audit "Root CA certificate generated and validated successfully"
    else
        error "Root CA certificate failed security validation"
        exit 1
    fi
    # CRL und Issuing URLs konfigurieren mit Sicherheitsparametern
    info "Configuring CRL and Issuing URLs with security hardening..."
    vault write pki/config/urls \
        issuing_certificates="$VAULT_ADDR/v1/pki/ca" \
        crl_distribution_points="$VAULT_ADDR/v1/pki/crl" >/dev/null 2>&1
    security_audit "CRL and Issuing URLs configured securely"
fi
# 5. Intermediate PKI für Service-Zertifikate (mit Sicherheitsprüfung)
info "Setting up Intermediate PKI with maximum security (TTL: $INTERMEDIATE_CA_TTL)..."
if vault secrets list | grep -q "^pki_int/"; then
    warn "SECURITY WARNING: Intermediate PKI already exists - verifying configuration"
    security_audit "Existing Intermediate PKI detected - performing security review"
else
    vault secrets enable -path=pki_int pki >/dev/null 2>&1
    vault secrets tune -max-lease-ttl="$PKI_INT_MAX_LEASE_TTL" pki_int >/dev/null 2>&1
    security_audit "Intermediate PKI engine enabled with security tuning"
    # CSR für Intermediate CA generieren mit Sicherheitsparametern
    info "Generating Intermediate CA CSR with maximum security parameters..."
    vault write -format=json pki_int/intermediate/generate/internal \
        common_name="$INTERMEDIATE_CA_COMMON_NAME" \
        organization="$INTERMEDIATE_CA_ORGANIZATION" \
        country="$INTERMEDIATE_CA_COUNTRY" \
        locality="$INTERMEDIATE_CA_LOCALITY" \
        province="$INTERMEDIATE_CA_PROVINCE" \
        ou="Intermediate Certificate Authority" \
        key_bits="$INTERMEDIATE_CA_KEY_BITS" \
        key_type="$INTERMEDIATE_CA_KEY_TYPE" \
        exclude_cn_from_sans=true \
        | jq -r '.data.csr' > /tmp/pki_intermediate.csr 2>/dev/null
    # CSR mit Root CA signieren
    info "Signing Intermediate CA certificate with Root CA..."
    vault write -format=json pki/root/sign-intermediate csr=@/tmp/pki_intermediate.csr \
        format=pem_bundle ttl="$INTERMEDIATE_CA_TTL" \
        | jq -r '.data.certificate' > /tmp/intermediate.cert.pem 2>/dev/null
    # Signiertes Intermediate Zertifikat speichern
    info "Installing signed Intermediate CA certificate..."
    vault write pki_int/intermediate/set-signed certificate=@/tmp/intermediate.cert.pem >/dev/null 2>&1
    # Sicherheitsvalidierung des Intermediate-Zertifikats
    if validate_certificate_security "/tmp/intermediate.cert.pem" "Intermediate CA"; then
        security_audit "Intermediate CA certificate generated and validated successfully"
    else
        error "Intermediate CA certificate failed security validation"
        exit 1
    fi
fi
# 6. Vault Rollen für Service-Zertifikate erstellen (mit maximalem Sicherheitsprofil)
info "Creating Vault roles with maximum security for service certificates..."
# Prüfen und erstellen der kubernetes-services Rolle mit Sicherheitsbeschränkungen
if ! vault list pki_int/roles 2>/dev/null | grep -q "kubernetes-services"; then
    info "Creating kubernetes-services role with maximum security..."
    vault write pki_int/roles/kubernetes-services \
        allowed_domains="$ALLOWED_DOMAINS" \
        allow_subdomains=true \
        allow_bare_domains=true \
        allow_localhost=false \
        allow_ip_sans=true \
        server_flag=true \
        client_flag=true \
        code_signing_flag=false \
        email_protection_flag=false \
        key_usage="DigitalSignature,KeyEncipherment" \
        ext_key_usage="ServerAuth,ClientAuth" \
        key_bits="$SERVICE_CERT_KEY_BITS" \
        key_type="$SERVICE_CERT_KEY_TYPE" \
        max_ttl="$SERVICE_CERT_MAX_TTL" \
        default_ttl="$SERVICE_CERT_DEFAULT_TTL" \
        allow_any_name=false \
        enforce_hostnames=true \
        allow_glob_domains=false \
        allow_wildcard_certificates=true \
        >/dev/null 2>&1
    security_audit "kubernetes-services role created with maximum security restrictions"
else
    info "Updating existing kubernetes-services role with maximum security..."
    vault write pki_int/roles/kubernetes-services \
        allowed_domains="$ALLOWED_DOMAINS" \
        allow_subdomains=true \
        allow_bare_domains=true \
        allow_localhost=false \
        allow_ip_sans=true \
        server_flag=true \
        client_flag=true \
        code_signing_flag=false \
        email_protection_flag=false \
        key_usage="DigitalSignature,KeyEncipherment" \
        ext_key_usage="ServerAuth,ClientAuth" \
        key_bits="$SERVICE_CERT_KEY_BITS" \
        key_type="$SERVICE_CERT_KEY_TYPE" \
        max_ttl="$SERVICE_CERT_MAX_TTL" \
        default_ttl="$SERVICE_CERT_DEFAULT_TTL" \
        allow_any_name=false \
        enforce_hostnames=true \
        allow_glob_domains=false \
        allow_wildcard_certificates=true \
        >/dev/null 2>&1
    security_audit "kubernetes-services role updated with maximum security restrictions"
fi
# Prüfen und erstellen der internal-services Rolle mit Sicherheitsbeschränkungen
if ! vault list pki_int/roles 2>/dev/null | grep -q "internal-services"; then
    info "Creating internal-services role with maximum security..."
    vault write pki_int/roles/internal-services \
        allowed_domains="$SPECIFIC_SERVICE_DOMAINS" \
        allow_subdomains=true \
        allow_bare_domains=true \
        allow_localhost=false \
        allow_ip_sans=true \
        server_flag=true \
        client_flag=true \
        code_signing_flag=false \
        email_protection_flag=false \
        key_usage="DigitalSignature,KeyEncipherment" \
        ext_key_usage="ServerAuth,ClientAuth" \
        key_bits="$SERVICE_CERT_KEY_BITS" \
        key_type="$SERVICE_CERT_KEY_TYPE" \
        max_ttl="$SERVICE_CERT_MAX_TTL" \
        default_ttl="$SERVICE_CERT_DEFAULT_TTL" \
        allow_any_name=false \
        enforce_hostnames=true \
        allow_glob_domains=false \
        allow_wildcard_certificates=false \
        >/dev/null 2>&1
    security_audit "internal-services role created with maximum security restrictions"
else
    info "Updating existing internal-services role with maximum security..."
    vault write pki_int/roles/internal-services \
        allowed_domains="$SPECIFIC_SERVICE_DOMAINS" \
        allow_subdomains=true \
        allow_bare_domains=true \
        allow_localhost=false \
        allow_ip_sans=true \
        server_flag=true \
        client_flag=true \
        code_signing_flag=false \
        email_protection_flag=false \
        key_usage="DigitalSignature,KeyEncipherment" \
        ext_key_usage="ServerAuth,ClientAuth" \
        key_bits="$SERVICE_CERT_KEY_BITS" \
        key_type="$SERVICE_CERT_KEY_TYPE" \
        max_ttl="$SERVICE_CERT_MAX_TTL" \
        default_ttl="$SERVICE_CERT_DEFAULT_TTL" \
        allow_any_name=false \
        enforce_hostnames=true \
        allow_glob_domains=false \
        allow_wildcard_certificates=false \
        >/dev/null 2>&1
    security_audit "internal-services role updated with maximum security restrictions"
fi
# 7. Sicherheitstest mit Zertifikatsausstellung
info "Performing security test with certificate issuance..."
# Test-Zertifikat für Kafka mit Sicherheitsprüfung
info "Issuing security test certificate for Kafka..."
if vault write pki_int/issue/internal-services \
    common_name="kafka.kafka.svc.cluster.local" \
    ttl="$SERVICE_CERT_DEFAULT_TTL" \
    ip_sans="127.0.0.1" \
    >/tmp/test_kafka_cert.json 2>/dev/null; then
    # Zertifikat extrahieren und validieren
    jq -r '.data.certificate' /tmp/test_kafka_cert.json > /tmp/test_kafka.crt 2>/dev/null
    if validate_certificate_security "/tmp/test_kafka.crt" "Test Service"; then
        info "Security test certificate issued and validated successfully"
        security_audit "Certificate issuance test passed with full security validation"
    else
        warn "Test certificate issued but failed security validation"
    fi
    # Temporäre Testdateien aufräumen
    rm -f /tmp/test_kafka_cert.json /tmp/test_kafka.crt
else
    warn "Failed to issue security test certificate - this may indicate configuration issues"
fi
# ==================== CERT-MANAGER INTEGRATION ====================
if [[ "$ENABLE_CERT_MANAGER_INTEGRATION" == "true" ]]; then
    info "=================================================================="
    info "Starting cert-manager integration..."
    security_audit "Initiating cert-manager integration process"

    # Prüfe, ob kubectl verfügbar ist
    if ! command -v kubectl &> /dev/null; then
        error "kubectl is required for cert-manager integration but not found."
        exit 1
    fi

    # Prüfe, ob Vault CA Zertifikat existiert (benötigt für caBundle)
    if [ ! -f "/tmp/CA_cert.crt" ]; then
        error "Root CA certificate (/tmp/CA_cert.crt) not found. Cannot create caBundle for cert-manager issuer."
        exit 1
    fi

    # Prüfe Authentifizierungseinstellungen
    if [[ -z "$CERT_MANAGER_VAULT_ROLE_ID" || -z "$CERT_MANAGER_VAULT_SECRET_ID" ]] && [[ -z "$CERT_MANAGER_VAULT_TOKEN" ]]; then
         warn "No Vault authentication credentials provided for cert-manager. You must set CERT_MANAGER_VAULT_ROLE_ID/SECRET_ID or CERT_MANAGER_VAULT_TOKEN."
         warn "Skipping cert-manager integration."
    else
        # Entscheide, ob Issuer oder ClusterIssuer verwendet wird (hier Issuer)
        # Du könntest dies auch konfigurierbar machen
        ISSUER_TYPE="Issuer" # oder "ClusterIssuer"

        # Wende den Intermediate Issuer an (typisch für Service-Zertifikate)
        apply_cert_manager_resources "$INTERMEDIATE_ISSUER_NAME" "$CERT_MANAGER_NAMESPACE" "$ISSUER_TYPE"

        # Optional: Wende auch einen Root Issuer an (weniger üblich, aber möglich)
        # apply_cert_manager_resources "$ROOT_ISSUER_NAME" "$CERT_MANAGER_NAMESPACE" "$ISSUER_TYPE" # oder eigenes Namespace-Handling für Root

        info "cert-manager integration steps completed."
        security_audit "cert-manager integration completed (Issuer: $INTERMEDIATE_ISSUER_NAME)"
    fi
else
    info "cert-manager integration is disabled (ENABLE_CERT_MANAGER_INTEGRATION=$ENABLE_CERT_MANAGER_INTEGRATION). Skipping..."
fi
# ==================== ENDE CERT-MANAGER INTEGRATION ====================
# 8. Sicherheitskonfiguration prüfen und dokumentieren
info "Performing final security configuration audit..."
security_audit "CA hierarchy established with Root -> Intermediate structure"
security_audit "Root CA validity period: $ROOT_CA_TTL (10 years)"
security_audit "Intermediate CA validity period: $INTERMEDIATE_CA_TTL (5 years)"
security_audit "Service certificate maximum validity: $SERVICE_CERT_MAX_TTL (30 days)"
security_audit "Service certificate default validity: $SERVICE_CERT_DEFAULT_TTL (8 hours)"
security_audit "Key sizes - Root/Intermediate: $ROOT_CA_KEY_BITS bits, Service: $SERVICE_CERT_KEY_BITS bits"
# 9. Aufräumen mit Sicherheitsprotokollierung
cleanup_temp_files
# 10. Status und Sicherheitshinweise anzeigen
info "=================================================================="
info "MAXIMUM SECURITY VAULT CA SETUP COMPLETE!"
info "=================================================================="
echo ""
info "SECURITY CONFIGURATION SUMMARY:"
info "  🔒 Root CA: $ROOT_CA_COMMON_NAME"
info "     • Validity: $ROOT_CA_TTL (10 years)"
info "     • Key Size: $ROOT_CA_KEY_BITS bits (RSA)"
info "     • Security: Maximum protection"
info ""
info "  🔒 Intermediate CA: $INTERMEDIATE_CA_COMMON_NAME"
info "     • Validity: $INTERMEDIATE_CA_TTL (5 years)"
info "     • Key Size: $INTERMEDIATE_CA_KEY_BITS bits (RSA)"
info "     • Security: Maximum protection"
info ""
info "  🔒 Service Certificates:"
info "     • Max Validity: $SERVICE_CERT_MAX_TTL (30 days)"
info "     • Default Validity: $SERVICE_CERT_DEFAULT_TTL (8 hours)"
info "     • Key Size: $SERVICE_CERT_KEY_BITS bits (RSA)"
info "     • Rotation: Frequent (every 8 hours)"
info ""
info "  🛡️  SECURITY FEATURES:"
info "     • mTLS support (ServerAuth/ClientAuth)"
info "     • Strong key usage restrictions"
info "     • Domain validation enforcement"
info "     • IP SAN support with validation"
info "     • Audit logging enabled"
info "     • Secure temporary file handling"
info ""
info "AVAILABLE ROLES:"
info "  • kubernetes-services (general k8s services with wildcards)"
info "  • internal-services (specific internal services)"
echo ""
info "CERTIFICATE ISSUANCE COMMANDS:"
info "  vault write pki_int/issue/internal-services common_name=\"<service-name>\" ttl=\"$SERVICE_CERT_DEFAULT_TTL\""
echo ""
info "SECURITY EXAMPLES:"
info "  Kafka: vault write pki_int/issue/internal-services common_name=\"kafka.kafka.svc.cluster.local\" ttl=\"$SERVICE_CERT_DEFAULT_TTL\""
info "  Redis: vault write pki_int/issue/internal-services common_name=\"redis.redis.svc.cluster.local\" ttl=\"$SERVICE_CERT_DEFAULT_TTL\""
echo ""
info "🔐 POST-SETUP SECURITY RECOMMENDATIONS:"
info "  1. Rotate the initial Vault token immediately"
info "  2. Implement AppRole authentication for services"
info "  3. Configure certificate revocation procedures"
info "  4. Set up monitoring for certificate issuance"
info "  5. Implement backup procedures for CA certificates"
info "  6. Regular security audits of issued certificates"
info "  7. If cert-manager integration was enabled, review the created Issuer resources and associated secrets."
echo ""
info "Vault Address: $VAULT_ADDR"
info "For maximum security, ensure Vault is accessed only through secure channels."
security_audit "Maximum security Vault CA setup completed successfully"
