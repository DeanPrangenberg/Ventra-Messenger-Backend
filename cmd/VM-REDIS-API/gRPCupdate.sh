#!/bin/bash

outPath="../shared"

if [ -d $outPath ]; then
    rm -f src/gRPC/*.pb.go
fi

mkdir -p $outPath

protoc \
    --go_out=$outPath \
    --go-grpc_out=$outPath \
    redis.proto
