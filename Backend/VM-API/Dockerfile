# Build stage: Compile Go binary
FROM golang:1.24.4 AS builder

WORKDIR /app

COPY go.mod ./
COPY go.sum ./
RUN go mod download

COPY src/ ./src/

WORKDIR /app/src
RUN CGO_ENABLED=0 GOOS=linux go build -a -installsuffix cgo -o /app/VM-API

# Runtime stage: Minimal image
FROM debian:stable-slim

WORKDIR /app

COPY --from=builder /app/VM-API .

EXPOSE 8881

CMD ["./VM-API"]