FROM golang:1.24.4 AS builder

WORKDIR /app

COPY .scripts/dev/generate-go-work.sh ./generate-go-work.sh
RUN chmod +x generate-go-work.sh && ./generate-go-work.sh VM-MD

COPY shared ./shared
COPY VM-MD ./VM-MD

WORKDIR /app/VM-MD

RUN go mod tidy && go mod download

RUN CGO_ENABLED=0 GOOS=linux go build -a -installsuffix cgo -o /app/VM-MD.bin /app/VM-MD/src/main.go

FROM debian:stable-slim

WORKDIR /app

RUN apt-get update && apt-get install -y curl jq && rm -rf /var/lib/apt/lists/*

COPY --from=builder /app/VM-MD.bin /usr/local/bin/vm-md
COPY .scripts/dev/entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

EXPOSE 8886

ENTRYPOINT ["/entrypoint.sh"]