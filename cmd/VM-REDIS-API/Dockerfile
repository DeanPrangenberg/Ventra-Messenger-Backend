FROM golang:1.24.4 AS builder

WORKDIR /app

COPY .scripts/dev/generate-go-work.sh ./generate-go-work.sh
RUN chmod +x generate-go-work.sh && ./generate-go-work.sh VM-REDIS-API

COPY shared ./shared
COPY VM-REDIS-API ./VM-REDIS-API
RUN ls -l ./VM-REDIS-API

WORKDIR /app/VM-REDIS-API

RUN go mod tidy && go mod download

RUN CGO_ENABLED=0 GOOS=linux go build -a -installsuffix cgo -o /app/VM-REDIS-API.bin /app/VM-REDIS-API/src/main.go

FROM debian:stable-slim

WORKDIR /app

RUN apt-get update && apt-get install -y curl jq && rm -rf /var/lib/apt/lists/*

COPY --from=builder /app/VM-REDIS-API.bin /usr/local/bin/vm-redis-api
COPY .scripts/dev/entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

EXPOSE 8886

ENTRYPOINT ["/entrypoint.sh"]