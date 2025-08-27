#!/bin/sh
SERVICE="$1"
cat > go.work <<EOF
go 1.24

use (
  ./pkg/CryptoLib
  ./pkg/Kafka-Wrapper
  ./pkg/Postgres-Wrapper
  ./pkg/Redis-Wrapper
  ./pkg/VM-API-gRPC-Wrapper
  ./cmd/$SERVICE
)
EOF