FROM golang:1.24.4 AS builder

WORKDIR /app

COPY scripts/dev/generate-go-work.sh ./generate-go-work.sh
RUN chmod +x generate-go-work.sh && ./generate-go-work.sh VM-API

COPY pkg ./pkg
COPY "cmd/VM-API" "./cmd/VM-API"

WORKDIR "/app/cmd/VM-API/src"

RUN go mod tidy && go mod download

RUN CGO_ENABLED=0 GOOS=linux go build -a -installsuffix cgo -o "/app/cmd/VM-API.bin" "/app/cmd/VM-API/src/main.go"

FROM debian:stable-slim

WORKDIR /app

RUN apt-get update && apt-get install -y curl jq && rm -rf /var/lib/apt/lists/*

COPY --from=builder "/app/cmd/VM-API.bin" /usr/local/bin/VM-API

RUN chmod +x /usr/local/bin/VM-API

ENTRYPOINT ["/usr/local/bin/VM-API"]

EXPOSE 8886