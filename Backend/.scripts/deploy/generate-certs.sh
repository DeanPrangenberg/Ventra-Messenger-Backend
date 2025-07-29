#!/bin/bash
set -euo pipefail

# === CONFIGURATION ===
TLS_DIR="./.vault/tls"
CA_DIR="$TLS_DIR/ca"

# === CLEANUP & CA GENERATION ===
echo "Removing existing certificates and keys..."
rm -rf "$TLS_DIR"
mkdir -p "$CA_DIR/private" "$CA_DIR/certs"

chmod 700 "$CA_DIR/private"
chmod 755 "$CA_DIR/certs"

echo "Generating Root CA private key..."
openssl genrsa -out "$CA_DIR/private/ca.key.pem" 4096
chmod 600 "$CA_DIR/private/ca.key.pem"
chown 100:100 "$CA_DIR/private/ca.key.pem" || true

echo "Generating Root CA certificate (ca.crt)..."
openssl req -x509 -new -key "$CA_DIR/private/ca.key.pem" \
  -days 3650 \
  -sha256 \
  -subj "/C=DE/ST=Berlin/L=Berlin/O=Ventra Messenger/CN=ca.ventra.internal" \
  -extensions v3_ca \
  -config <(cat /etc/ssl/openssl.cnf <(printf "\n[v3_ca]\nbasicConstraints=CA:true\nkeyUsage=keyCertSign,cRLSign")) \
  -out "$TLS_DIR/ca.crt"
chmod 644 "$TLS_DIR/ca.crt"
chown 100:100 "$TLS_DIR/ca.crt" || true

# === VAULT SERVER CERT ===
echo "Generating Vault server key..."
openssl genrsa -out "$TLS_DIR/vault.key" 4096
chmod 600 "$TLS_DIR/vault.key"
chown 100:100 "$TLS_DIR/vault.key" || true

echo "Creating Vault server CSR..."
openssl req -new -key "$TLS_DIR/vault.key" \
  -subj "/C=DE/ST=Berlin/L=Berlin/O=Ventra Messenger/CN=vm-vault" \
  -out "$TLS_DIR/vault.csr"

echo "Signing Vault server certificate with Root CA..."
openssl x509 -req -in "$TLS_DIR/vault.csr" \
  -CA "$TLS_DIR/ca.crt" \
  -CAkey "$CA_DIR/private/ca.key.pem" \
  -CAcreateserial \
  -days 365 \
  -sha256 \
  -extfile <(printf "subjectAltName=DNS:vm-vault,DNS:localhost,IP:127.0.0.1") \
  -out "$TLS_DIR/vault.crt"
chmod 644 "$TLS_DIR/vault.crt"
chown 100:100 "$TLS_DIR/vault.crt" || true

rm -f "$TLS_DIR/vault.csr"

echo "All CA and Vault server certificates generated."
echo "Root CA: $TLS_DIR/ca.crt"
echo "Vault server: $TLS_DIR/vault.crt, $TLS_DIR/vault.key"