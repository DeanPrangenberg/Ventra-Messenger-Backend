#!/bin/sh

set -e

VAULT_ADDR="${VAULT_ADDR:-https://vm-vault:8200}"
VAULT_TOKEN="${VAULT_TOKEN:?Set VAULT_TOKEN as env or secret}"

CA_PEM="/tmp/ca.pem"
CLIENT_PEM="/tmp/client.pem"
CLIENT_KEY="/tmp/client.key"
KEYSTORE_JKS="/tmp/client.keystore.jks"
TRUSTSTORE_JKS="/tmp/client.truststore.jks"
KEYSTORE_PASS="changeit"
TRUSTSTORE_PASS="changeit"

vault login --no-print "$VAULT_TOKEN"

vault write -format=json pki-int/issue/init-cert-role \
  common_name="init-bootstrap.services.ventra.internal" \
  ttl="5m" > /tmp/cert.json

jq -r .data.certificate /tmp/cert.json > "$CLIENT_PEM"
jq -r .data.issuing_ca /tmp/cert.json > "$CA_PEM"
jq -r .data.private_key /tmp/cert.json > "$CLIENT_KEY"

# Convert PEM to PKCS12
openssl pkcs12 -export \
  -in "$CLIENT_PEM" \
  -inkey "$CLIENT_KEY" \
  -certfile "$CA_PEM" \
  -out /tmp/client.p12 \
  -password pass:$KEYSTORE_PASS

# Import PKCS12 to JKS keystore
keytool -importkeystore \
  -deststorepass $KEYSTORE_PASS \
  -destkeystore "$KEYSTORE_JKS" \
  -srckeystore /tmp/client.p12 \
  -srcstoretype PKCS12 \
  -srcstorepass $KEYSTORE_PASS \
  -alias 1

# Create JKS truststore
keytool -import -trustcacerts -noprompt \
  -alias vaultca \
  -file "$CA_PEM" \
  -keystore "$TRUSTSTORE_JKS" \
  -storepass $TRUSTSTORE_PASS

BROKER="kafka:9092"
TOPICS="messages user-status user-auth-event stats-updates key-rotation"

CONFIG_FILE="/tmp/kafka_ssl_config.properties"
cat > "$CONFIG_FILE" <<EOF
security.protocol=SSL
ssl.keystore.location=$KEYSTORE_JKS
ssl.keystore.password=$KEYSTORE_PASS
ssl.key.password=$KEYSTORE_PASS
ssl.truststore.location=$TRUSTSTORE_JKS
ssl.truststore.password=$TRUSTSTORE_PASS
EOF

while ! (exec 3<>/dev/tcp/kafka/9092) 2>/dev/null; do
  echo "Waiting for Kafka..."
  sleep 3
done

for TOPIC in $TOPICS; do
  if [ -z "$TOPIC" ]; then
    continue
  fi
  kafka-topics.sh \
    --bootstrap-server "$BROKER" \
    --create \
    --topic "$TOPIC" \
    --partitions 1 \
    --replication-factor 1 \
    --command-config "$CONFIG_FILE" \
    && echo "Topic $TOPIC created" \
    || echo "Could not create topic $TOPIC"
done

rm -f "$CONFIG_FILE" "$CA_PEM" "$CLIENT_PEM" "$CLIENT_KEY" /tmp/cert.json /tmp/client.p12 "$KEYSTORE_JKS" "$TRUSTSTORE_JKS"