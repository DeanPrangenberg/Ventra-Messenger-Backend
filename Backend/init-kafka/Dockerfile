FROM golang:1.21-alpine as builder

WORKDIR /app
COPY . .
RUN go mod download
RUN go build -o init-kafka .

FROM alpine:3.19
COPY --from=builder /app/init-kafka /init-kafka
ENTRYPOINT ["/init-kafka"]
