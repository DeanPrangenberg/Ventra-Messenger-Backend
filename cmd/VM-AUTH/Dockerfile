FROM golang:1.24.4 AS builder

WORKDIR /app

COPY scripts/dev/generate-go-work.sh ./generate-go-work.sh
RUN chmod +x generate-go-work.sh && ./generate-go-work.sh VM-AUTH

COPY shared ./shared
COPY VM-AUTH ./VM-AUTH

WORKDIR /app/VM-AUTH/src

RUN go mod tidy && go mod download

RUN CGO_ENABLED=0 GOOS=linux go build -a -installsuffix cgo -o /app/VM-AUTH.bin /app/VM-AUTH/src/main.go

FROM debian:stable-slim

WORKDIR /app

RUN apt-get update && apt-get install -y curl jq && rm -rf /var/lib/apt/lists/*

COPY --from=builder /app/VM-AUTH.bin /usr/local/bin/vm-auth
COPY scripts/dev/entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

EXPOSE 8886

ENTRYPOINT ["/entrypoint.sh"]