FROM golang:1.24.4 AS builder

WORKDIR /app/Backend/VM-REDIS-API

COPY Backend/VM-REDIS-API/ ./
COPY Backend/shared ../../shared

RUN go mod tidy && go mod download

WORKDIR src
RUN CGO_ENABLED=0 GOOS=linux go build -o /app/VM-REDIS-API

FROM debian:stable-slim

WORKDIR /app

COPY --from=builder /app/VM-REDIS-API .

EXPOSE 8881

CMD ["./VM-REDIS-API"]