FROM golang:1.24.4 AS builder

WORKDIR /app

COPY Backend/generate-go-work.sh ./
RUN chmod +x generate-go-work.sh && ./generate-go-work.sh VM-LOGGER

COPY Backend/shared ./shared
COPY Backend/VM-LOGGER ./VM-LOGGER

WORKDIR /app/VM-LOGGER/src

RUN go mod tidy
RUN go mod download

RUN CGO_ENABLED=0 GOOS=linux go build -a -installsuffix cgo -o /app/VM-LOGGER.bin

FROM debian:stable-slim

WORKDIR /app

RUN apt-get update && apt-get install -y curl jq && rm -rf /var/lib/apt/lists/*

COPY --from=builder /app/VM-LOGGER.bin /usr/local/bin/vm-logger
COPY Backend/entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

EXPOSE 8886

ENTRYPOINT ["/entrypoint.sh"]