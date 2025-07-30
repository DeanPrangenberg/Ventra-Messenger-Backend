#!/bin/bash
# setup-vault-ca-secure.sh - Maximum Security Production Vault CA Setup with cert-manager Integration

# ==================== CONFIGURATION ====================
ROOT_CA_COMMON_NAME="Ventra Production Root CA"
ROOT_CA_ORGANIZATION="Ventra Security Operations"
ROOT_CA_TTL="87600h"
ROOT_CA_KEY_BITS="4096"
ROOT_CA_KEY_TYPE="rsa"
ROOT_CA_COUNTRY="DE"
ROOT_CA_LOCALITY="Berlin"
ROOT_CA_PROVINCE="Berlin"
ROOT_CA_STREET_ADDRESS="Secure Operations Center"
ROOT_CA_POSTAL_CODE="10115"

INTERMEDIATE_CA_COMMON_NAME="Ventra Production Services Intermediate Authority"
INTERMEDIATE_CA_ORGANIZATION="Ventra Certificate Services"
INTERMEDIATE_CA_TTL="43800h"
INTERMEDIATE_CA_KEY_BITS="4096"
INTERMEDIATE_CA_KEY_TYPE="rsa"
INTERMEDIATE_CA_COUNTRY="DE"
INTERMEDIATE_CA_LOCALITY="Berlin"
INTERMEDIATE_CA_PROVINCE="Berlin"

SERVICE_CERT_MAX_TTL="720h"
SERVICE_CERT_DEFAULT_TTL="8h"
SERVICE_CERT_KEY_BITS="2048"
SERVICE_CERT_KEY_TYPE="rsa"

ALLOWED_DOMAINS="svc.cluster.local,cluster.local,internal.ventra.local"
SPECIFIC_SERVICE_DOMAINS="kafka.kafka.svc.cluster.local,redis.redis.svc.cluster.local,postgres-postgresql.database.svc.cluster.local,vault.vault.svc.cluster.local"

PKI_MAX_LEASE_TTL="87600h"
PKI_INT_MAX_LEASE_TTL="43800h"

VAULT_ADDR="${VAULT_ADDR:-http://127.0.0.1:30443}"

# Kubernetes secret config for Vault token
VAULT_SECRET_NAME="${VAULT_SECRET_NAME:-vault-init-secret}"
VAULT_SECRET_NAMESPACE="${VAULT_SECRET_NAMESPACE:-vault}"

ENABLE_CERT_MANAGER_INTEGRATION="${ENABLE_CERT_MANAGER_INTEGRATION:-false}"
CERT_MANAGER_VAULT_ADDR="${CERT_MANAGER_VAULT_ADDR:-$VAULT_ADDR}"
CERT_MANAGER_VAULT_ROLE_ID="${CERT_MANAGER_VAULT_ROLE_ID:-}"
CERT_MANAGER_VAULT_SECRET_ID="${CERT_MANAGER_VAULT_SECRET_ID:-}"
CERT_MANAGER_NAMESPACE="${CERT_MANAGER_NAMESPACE:-cert-manager}"
ROOT_ISSUER_NAME="${ROOT_ISSUER_NAME:-vault-root-issuer}"
INTERMEDIATE_ISSUER_NAME="${INTERMEDIATE_ISSUER_NAME:-vault-intermediate-issuer}"
ROOT_PKI_PATH="${ROOT_PKI_PATH:-pki}"
INTERMEDIATE_PKI_PATH="${INTERMEDIATE_PKI_PATH:-pki_int}"

LOG_LEVEL="${LOG_LEVEL:-INFO}"
CLEANUP_TEMP_FILES="${CLEANUP_TEMP_FILES:-true}"
ENABLE_AUDIT_LOGGING="${ENABLE_AUDIT_LOGGING:-true}"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BACKEND_ROOT_DIR="$SCRIPT_DIR/../../.."
source "$BACKEND_ROOT_DIR/.scripts/functions/logs.sh"
TMP_DIR="$BACKEND_ROOT_DIR/.scripts/deploy/tmp"

# ==================== SECURITY FUNCTIONS ====================
security_audit() {
  echo -e "${CYAN}[AUDIT]${NC} $1"
}

handle_error() {
    local line_number="$1"
    local error_code="$2"
    error "CRITICAL SECURITY ERROR at line $line_number with exit code $error_code"
    security_audit "Setup process terminated due to security error"
    cleanup_temp_files
    exit "$error_code"
}

cleanup_temp_files() {
    if [[ "$CLEANUP_TEMP_FILES" == "true" ]]; then
        log "Performing secure cleanup of temporary files..."
        for file in $TMP_DIR/CA_cert.crt $TMP_DIR/pki_intermediate.csr $TMP_DIR/intermediate.cert.pem $TMP_DIR/root_ca.json $TMP_DIR/int_ca.json $TMP_DIR/test_kafka_cert.json $TMP_DIR/test_kafka.crt $TMP_DIR/*-issuer.yaml; do
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

fetch_vault_token_from_secret() {
    log "Fetching Vault token from Kubernetes secret '$VAULT_SECRET_NAME' in namespace '$VAULT_SECRET_NAMESPACE'..."
    VAULT_TOKEN=$(kubectl get secret "$VAULT_SECRET_NAME" --namespace "$VAULT_SECRET_NAMESPACE" -o jsonpath="{.data.root-token}" | base64 -d)
    if [[ -z "$VAULT_TOKEN" ]]; then
        error "Failed to fetch Vault token from secret"
        exit 1
    fi
    export VAULT_TOKEN
    security_audit "Vault token loaded from Kubernetes secret"
}

validate_vault_token() {
    log "Validating Vault token security..."
    local token_info
    token_info=$(vault token lookup 2>/dev/null) || {
        error "Cannot lookup Vault token - security validation failed"
        return 1
    }
    if echo "$token_info" | grep -q "root"; then
        security_audit "Using root token for initial setup (expected for CA initialization)"
    else
        log_warn "Using non-root token - some operations may fail"
    fi
    local ttl
    ttl=$(echo "$token_info" | grep "ttl" | awk '{print $2}')
    if [[ "$ttl" == "0" ]] || [[ -z "$ttl" ]]; then
        security_audit "Using non-expiring token - ensure proper token management after setup"
    else
        security_audit "Token has TTL: $ttl - automatic expiration enabled"
    fi
}

validate_certificate_security() {
    local cert_file="$1"
    local cert_type="$2"
    if [ ! -f "$cert_file" ]; then
        error "Certificate file $cert_file not found for security validation"
        return 1
    fi
    log "Performing security validation of $cert_type certificate..."
    local key_size
    key_size=$(openssl x509 -in "$cert_file" -noout -text | grep -E "RSA Public-Key|Public-Key" | grep -o "([0-9]* bit)" | grep -o "[0-9]*")
    if [ -z "$key_size" ]; then
        error "Could not extract key size from $cert_type certificate"
        return 1
    fi
    if [ "$key_size" -lt 2048 ]; then
        error "Certificate $cert_type has insecure key size: $key_size bits (minimum 2048 required)"
        return 1
    fi
    local sig_alg
    sig_alg=$(openssl x509 -in "$cert_file" -noout -text | grep "Signature Algorithm" | head -1 | awk '{print $3}')
    if [[ ! "$sig_alg" =~ ^(sha256WithRSAEncryption|sha384WithRSAEncryption|sha512WithRSAEncryption)$ ]]; then
        error "Certificate $cert_type uses weak signature algorithm: $sig_alg"
        return 1
    fi
    security_audit "$cert_type certificate security validation passed (Key Size: $key_size, Sig Alg: $sig_alg)"
    return 0
}

# ==================== CERT-MANAGER FUNCTIONS ====================
create_vault_issuer_yaml() {
    local issuer_name="$1"
    local vault_path="$2"
    local issuer_namespace="$3"
    local issuer_type="${4:-Issuer}"
    local output_file="$5"
    local auth_config=""
    if [[ -n "$CERT_MANAGER_VAULT_ROLE_ID" && -n "$CERT_MANAGER_VAULT_SECRET_ID" ]]; then
        auth_config="appRole:
        path: approle
        roleId: \"$CERT_MANAGER_VAULT_ROLE_ID\"
        secretRef:
          name: \"${issuer_name}-vault-secret\"
          key: secretId"
    elif [[ -n "$CERT_MANAGER_VAULT_TOKEN" ]]; then
        auth_config="tokenSecretRef:
        name: \"${issuer_name}-vault-token\"
        key: token"
    else
        error "Either AppRole credentials or Vault Token must be provided for cert-manager integration."
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
    caBundle: $(base64 -w 0 $TMP_DIR/CA_cert.crt)
    auth:
      $auth_config
EOF
    log "Created $issuer_type YAML for '$issuer_name' at $output_file"
}

apply_cert_manager_resources() {
    local issuer_name="$1"
    local issuer_namespace="$2"
    local issuer_type="$3"
    local vault_secret_name="${issuer_name}-vault-secret"
    local vault_token_secret_name="${issuer_name}-vault-token"
    local issuer_yaml_file="$TMP_DIR/${issuer_name}-issuer.yaml"
    log "Applying cert-manager resources for $issuer_type '$issuer_name'..."
    if [[ -n "$CERT_MANAGER_VAULT_ROLE_ID" && -n "$CERT_MANAGER_VAULT_SECRET_ID" ]]; then
        log "Creating AppRole Secret for '$issuer_name'..."
        kubectl create secret generic "$vault_secret_name" \
            --namespace="$issuer_namespace" \
            --from-literal=secretId="$CERT_MANAGER_VAULT_SECRET_ID" \
            --dry-run=client -o yaml | kubectl apply -f -
        security_audit "Created AppRole secret '$vault_secret_name' for issuer '$issuer_name'"
    elif [[ -n "$CERT_MANAGER_VAULT_TOKEN" ]]; then
        log "Creating Token Secret for '$issuer_name'..."
        kubectl create secret generic "$vault_token_secret_name" \
            --namespace="$issuer_namespace" \
            --from-literal=token="$CERT_MANAGER_VAULT_TOKEN" \
            --dry-run=client -o yaml | kubectl apply -f -
        security_audit "Created Token secret '$vault_token_secret_name' for issuer '$issuer_name'"
    fi
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
    log "Checking status of $issuer_type '$issuer_name'..."
    sleep 5
    local issuer_status
    issuer_status=$(kubectl get "$issuer_type" "$issuer_name" --namespace="$issuer_namespace" -o jsonpath='{.status.conditions[?(@.type=="Ready")].status}' 2>/dev/null)
    if [[ "$issuer_status" == "True" ]]; then
        log "$issuer_type '$issuer_name' is Ready."
    else
         log_warn "$issuer_type '$issuer_name' might not be Ready yet. Check status with: kubectl get $issuer_type $issuer_name --namespace=$issuer_namespace"
    fi
}

# ==================== MAIN SCRIPT ====================
log "Initializing Maximum Security Vault Certificate Authority Setup..."
trap 'handle_error $LINENO $?' ERR
set -eE

fetch_vault_token_from_secret

log "Checking Vault accessibility and security posture..."
counter=0; max_retries=30; retry_interval=2
while [ $counter -lt $max_retries ]; do
    if curl -s -o /dev/null -w "%{http_code}" "$VAULT_ADDR/v1/sys/health" 2>/dev/null | grep -q "200\|429"; then
        log "Vault is accessible and responding"
        break
    fi
    log_warn "Waiting for Vault to be ready... ($((counter+1))/$max_retries)"
    sleep $retry_interval
    counter=$((counter + 1))
done
if [ $counter -eq $max_retries ]; then
    error "Vault is not accessible after $((max_retries * retry_interval)) seconds - aborting for security"
    exit 1
fi

validate_vault_token

if [[ "$ENABLE_AUDIT_LOGGING" == "true" ]]; then
    log "Enabling Vault audit logging for security monitoring..."
    if ! vault audit list | grep -q "file/"; then
        vault audit enable file file_path=/vault/logs/vault-audit.log
        security_audit "Audit logging enabled for CA operations"
    else
        security_audit "Audit logging already enabled"
    fi
fi

log "Checking existing PKI configuration for security conflicts..."
if vault secrets list | grep -q "^pki/"; then
    log_warn "SECURITY WARNING: PKI secret engine already exists - verifying configuration"
    security_audit "Existing PKI configuration detected - performing security review"
else
    log "Enabling PKI secret engine with maximum security settings..."
    vault secrets enable pki >/dev/null 2>&1
    security_audit "PKI secret engine enabled"
    log "Tuning PKI secret engine for maximum security (max lease TTL: $PKI_MAX_LEASE_TTL)..."
    vault secrets tune -max-lease-ttl="$PKI_MAX_LEASE_TTL" pki >/dev/null 2>&1
    security_audit "PKI engine tuned for maximum security parameters"
    log "Generating Root CA certificate with maximum security (TTL: $ROOT_CA_TTL, Key Size: $ROOT_CA_KEY_BITS)..."
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
        > $TMP_DIR/CA_cert.crt 2>/dev/null
    if validate_certificate_security "$TMP_DIR/CA_cert.crt" "Root CA"; then
        security_audit "Root CA certificate generated and validated successfully"
    else
        error "Root CA certificate failed security validation"
        exit 1
    fi
    log "Configuring CRL and Issuing URLs with security hardening..."
    vault write pki/config/urls \
        issuing_certificates="$VAULT_ADDR/v1/pki/ca" \
        crl_distribution_points="$VAULT_ADDR/v1/pki/crl" >/dev/null 2>&1
    security_audit "CRL and Issuing URLs configured securely"
fi

log "Setting up Intermediate PKI with maximum security (TTL: $INTERMEDIATE_CA_TTL)..."
if vault secrets list | grep -q "^pki_int/"; then
    log_warn "SECURITY WARNING: Intermediate PKI already exists - verifying configuration"
    security_audit "Existing Intermediate PKI detected - performing security review"
else
    vault secrets enable -path=pki_int pki >/dev/null 2>&1
    vault secrets tune -max-lease-ttl="$PKI_INT_MAX_LEASE_TTL" pki_int >/dev/null 2>&1
    security_audit "Intermediate PKI engine enabled with security tuning"
    log "Generating Intermediate CA CSR with maximum security parameters..."
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
        | jq -r '.data.csr' > $TMP_DIR/pki_intermediate.csr 2>/dev/null
    log "Signing Intermediate CA certificate with Root CA..."
    vault write -format=json pki/root/sign-intermediate csr=@$TMP_DIR/pki_intermediate.csr \
        format=pem_bundle ttl="$INTERMEDIATE_CA_TTL" \
        | jq -r '.data.certificate' > $TMP_DIR/intermediate.cert.pem 2>/dev/null
    log "Installing signed Intermediate CA certificate..."
    vault write pki_int/intermediate/set-signed certificate=@$TMP_DIR/intermediate.cert.pem >/dev/null 2>&1
    if validate_certificate_security "$TMP_DIR/intermediate.cert.pem" "Intermediate CA"; then
        security_audit "Intermediate CA certificate generated and validated successfully"
    else
        error "Intermediate CA certificate failed security validation"
        exit 1
    fi
fi

log "Creating Vault roles with maximum security for service certificates..."
if ! vault list pki_int/roles 2>/dev/null | grep -q "kubernetes-services"; then
    log "Creating kubernetes-services role with maximum security..."
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
    log "Updating existing kubernetes-services role with maximum security..."
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

if ! vault list pki_int/roles 2>/dev/null | grep -q "internal-services"; then
    log "Creating internal-services role with maximum security..."
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
    log "Updating existing internal-services role with maximum security..."
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

log "Performing security test with certificate issuance..."
log "Issuing security test certificate for Kafka..."
if vault write -format=json pki_int/issue/internal-services \
    common_name="kafka.kafka.svc.cluster.local" \
    ttl="$SERVICE_CERT_DEFAULT_TTL" \
    ip_sans="127.0.0.1" \
    >$TMP_DIR/test_kafka_cert.json 2>/dev/null; then
    jq -r '.data.certificate' $TMP_DIR/test_kafka_cert.json > $TMP_DIR/test_kafka.crt 2>/dev/null
    if validate_certificate_security "$TMP_DIR/test_kafka.crt" "Test Service"; then
        log "Security test certificate issued and validated successfully"
        security_audit "Certificate issuance test passed with full security validation"
    else
        log_warn "Test certificate issued but failed security validation"
    fi
    rm -f $TMP_DIR/test_kafka_cert.json $TMP_DIR/test_kafka.crt
else
    log_warn "Failed to issue security test certificate - this may indicate configuration issues"
fi

if [[ "$ENABLE_CERT_MANAGER_INTEGRATION" == "true" ]]; then
    log "=================================================================="
    log "Starting cert-manager integration..."
    security_audit "Initiating cert-manager integration process"
    if ! command -v kubectl &> /dev/null; then
        error "kubectl is required for cert-manager integration but not found."
        exit 1
    fi
    if [ ! -f "$TMP_DIR/CA_cert.crt" ]; then
        error "Root CA certificate ($TMP_DIR/CA_cert.crt) not found. Cannot create caBundle for cert-manager issuer."
        exit 1
    fi
    if [[ -z "$CERT_MANAGER_VAULT_ROLE_ID" || -z "$CERT_MANAGER_VAULT_SECRET_ID" ]] && [[ -z "$CERT_MANAGER_VAULT_TOKEN" ]]; then
         log_warn "No Vault authentication credentials provided for cert-manager. You must set CERT_MANAGER_VAULT_ROLE_ID/SECRET_ID or CERT_MANAGER_VAULT_TOKEN."
         log_warn "Skipping cert-manager integration."
    else
        ISSUER_TYPE="Issuer"
        apply_cert_manager_resources "$INTERMEDIATE_ISSUER_NAME" "$CERT_MANAGER_NAMESPACE" "$ISSUER_TYPE"
        log "cert-manager integration steps completed."
        security_audit "cert-manager integration completed (Issuer: $INTERMEDIATE_ISSUER_NAME)"
    fi
else
    log "cert-manager integration is disabled (ENABLE_CERT_MANAGER_INTEGRATION=$ENABLE_CERT_MANAGER_INTEGRATION). Skipping..."
fi

log "Performing final security configuration audit..."
security_audit "CA hierarchy established with Root -> Intermediate structure"
security_audit "Root CA validity period: $ROOT_CA_TTL (10 years)"
security_audit "Intermediate CA validity period: $INTERMEDIATE_CA_TTL (5 years)"
security_audit "Service certificate maximum validity: $SERVICE_CERT_MAX_TTL (30 days)"
security_audit "Service certificate default validity: $SERVICE_CERT_DEFAULT_TTL (8 hours)"
security_audit "Key sizes - Root/Intermediate: $ROOT_CA_KEY_BITS bits, Service: $SERVICE_CERT_KEY_BITS bits"

log "=================================================================="
log "MAXIMUM SECURITY VAULT CA SETUP COMPLETE!"
log "=================================================================="