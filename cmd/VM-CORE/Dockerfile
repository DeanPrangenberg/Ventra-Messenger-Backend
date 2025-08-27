FROM golang:1.24.4 AS builder

WORKDIR /app

COPY scripts/dev/generate-go-work.sh ./generate-go-work.sh
RUN chmod +x generate-go-work.sh && ./generate-go-work.sh VM-CORE

COPY pkg ./pkg
COPY "cmd/VM-CORE" "./cmd/VM-CORE"

WORKDIR "/app/cmd/VM-CORE/src"

RUN go mod tidy && go mod download

RUN CGO_ENABLED=0 GOOS=linux go build -a -installsuffix cgo -o "/app/cmd/VM-CORE.bin" "/app/cmd/VM-CORE/src/main.go"

FROM debian:stable-slim

WORKDIR /app

RUN apt-get update && apt-get install -y curl jq && rm -rf /var/lib/apt/lists/*

COPY --from=builder "/app/cmd/VM-CORE.bin" /usr/local/bin/vm-core
RUN chmod +x /usr/local/bin/vm-core

ENTRYPOINT ["/usr/local/bin/vm-core"]