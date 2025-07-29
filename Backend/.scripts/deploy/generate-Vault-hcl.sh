#!/bin/sh
set -eu

DATA_DIR=".vault/data"
mkdir -p "$DATA_DIR"
chown 100:100 "$DATA_DIR"
chmod 700 "$DATA_DIR"

CONFIG_DIR=".vault/config"
CONFIG_FILE="$CONFIG_DIR/vault.hcl"

mkdir -p "$CONFIG_DIR"

cat > "$CONFIG_FILE" <<EOF
storage "file" {
  path = "/.vault/data"
}

listener "tcp" {
  address     = "0.0.0.0:8200"
  tls_cert_file = "/.vault/tls/vault.crt"
  tls_key_file  = "/.vault/tls/vault.key"
  tls_client_ca_file = "/.vault/tls/ca.crt"
}

ui = true
disable_mlock = false
EOF

echo "Vault config written to $CONFIG_FILE"