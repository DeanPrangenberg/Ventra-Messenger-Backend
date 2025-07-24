#!/bin/bash

if [ -d "src/gRPC" ]; then
    rm -f src/gRPC/*.pb.go
fi

mkdir -p src/gRPC

protoc \
    --go_out=src/gRPC \
    --go-grpc_out=src/gRPC \
    *.proto
