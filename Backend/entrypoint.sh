#!/bin/sh
set -u

SERVICE_NAME="${SERVICE_NAME:-unknown}"

VAULT_ADDR="https://vm-vault:8200"
CERT_PATH="/tmp/ventra/client.crt"
KEY_PATH="/tmp/ventra/client.key"
CA_PATH="/tmp/ventra/ca.crt"
VAULT_PKI_PATH="pki-int/issue/service-mtls"

GEN_DIR="/tmp/ventra-gen"
GEN_CERT_PATH="$GEN_DIR/client.crt"
GEN_KEY_PATH="$GEN_DIR/client.key"
GEN_CA_PATH="$GEN_DIR/ca.crt"

TMP_TOKEN_JSON="/tmp/vault_token_response.json"
TMP_CERT_JSON="/tmp/vault_cert_response.json"

log() { echo "[entrypoint][$SERVICE_NAME] $*"; }

TZ="${TZ:-UTC}"
export TZ
log "Timezone set to $TZ"

if command -v ntpd >/dev/null 2>&1; then
  log "Syncing time with ntpd..."
  ntpd -q -p pool.ntp.org || log "ntpd failed, continuing anyway"
else
  log "ntpd not found, skipping time sync"
fi

log "Current date/time: $(date -u)"

mkdir -p "$GEN_DIR"

for f in "$CERT_PATH" "$KEY_PATH" "$CA_PATH"; do
  while [ ! -s "$f" ]; do
    log "Waiting for $f to exist and be non-empty..."
    sleep 2
  done
done

while :; do
  if curl -sk --cacert "$CA_PATH" "$VAULT_ADDR/v1/sys/health" | grep -q '"initialized":true'; then
    log "Vault is reachable and initialized."
    break
  else
    log "Vault not reachable or not initialized yet."
    sleep 2
  fi
done

while :; do
  log "Requesting 1min token from Vault at $(date -u)"
  curl -sk \
    --cert "$CERT_PATH" \
    --key "$KEY_PATH" \
    --cacert "$CA_PATH" \
    -X POST \
    -d '{"ttl": "1m"}' \
    "$VAULT_ADDR/v1/auth/cert/login" > "$TMP_TOKEN_JSON" 2>/dev/null

  if jq -e . "$TMP_TOKEN_JSON" >/dev/null 2>&1; then
    VAULT_TOKEN=$(jq -r '.auth.client_token // empty' "$TMP_TOKEN_JSON")
    if [ -n "$VAULT_TOKEN" ]; then
      log "Successfully obtained 1min token from Vault."
      break
    fi
  fi
  log "Failed to obtain token from Vault. Response: $(cat $TMP_TOKEN_JSON)"
  sleep 5
done

while :; do
  log "Requesting certificate from Vault at $(date -u)"
  curl -sk \
    --header "X-Vault-Token: $VAULT_TOKEN" \
    --cacert "$CA_PATH" \
    -X POST \
    -d "{\"common_name\": \"$(hostname).services.ventra.internal\", \"ttl\": \"24h\"}" \
    "$VAULT_ADDR/v1/$VAULT_PKI_PATH" > "$TMP_CERT_JSON" 2>/dev/null

  if jq -e '.data.certificate and .data.private_key and .data.issuing_ca' "$TMP_CERT_JSON" >/dev/null 2>&1; then
    jq -r '.data.certificate' "$TMP_CERT_JSON" > "$GEN_CERT_PATH"
    jq -r '.data.private_key' "$TMP_CERT_JSON" > "$GEN_KEY_PATH"
    jq -r '.data.issuing_ca' "$TMP_CERT_JSON" > "$GEN_CA_PATH"
    log "Successfully obtained new certificate from Vault and saved to $GEN_DIR."
    break
  fi
  log "Failed to obtain certificate from Vault. Response: $(cat $TMP_CERT_JSON)"
  sleep 5
done

while :; do
  log "Starting service: /usr/local/bin/$SERVICE_NAME"
  /usr/local/bin/$SERVICE_NAME || log "Service exited with code $?; restarting..."
  sleep 2
done