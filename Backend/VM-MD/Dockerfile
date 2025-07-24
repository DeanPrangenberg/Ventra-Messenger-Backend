FROM golang:1.24.4 AS builder

WORKDIR /app

COPY Backend/VM-MD/go.mod ./
RUN go mod tidy

COPY Backend/shared ../shared
COPY Backend/VM-MD/src/ ./src/

RUN go mod download

COPY Backend/VM-MD/src/ ./src/

WORKDIR /app/src
RUN CGO_ENABLED=0 GOOS=linux go build -a -installsuffix cgo -o /app/VM-MD

FROM debian:stable-slim

WORKDIR /app

COPY --from=builder /app/VM-MD .

EXPOSE 8881

CMD ["./VM-MD"]