FROM golang:1.24.4 AS builder

WORKDIR /app

COPY Backend/VM-API/go.mod Backend/VM-API/go.sum ./
COPY Backend/shared/redisAPI ../shared/redisAPI
RUN go mod download

COPY Backend/VM-API/src/ ./src/

WORKDIR /app/src
RUN CGO_ENABLED=0 GOOS=linux go build -a -installsuffix cgo -o /app/VM-API

FROM debian:stable-slim

WORKDIR /app

COPY --from=builder /app/VM-API .

EXPOSE 8881

CMD ["./VM-API"]