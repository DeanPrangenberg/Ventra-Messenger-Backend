#!/bin/sh
SERVICE="$1"
cat > go.work <<EOF
go 1.24

use (
  ./shared/CryptoLib
  ./shared/Kafka-Wrapper
  ./shared/Logger-Wrapper
  ./shared/Postgres-Wrapper
  ./shared/Redis-API-Wrapper
  ./$SERVICE
)
EOF