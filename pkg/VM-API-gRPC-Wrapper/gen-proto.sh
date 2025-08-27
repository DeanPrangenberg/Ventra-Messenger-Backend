#!/bin/bash

rm -rf gen-pb/*

protoc --go_out=../ --go-grpc_out=../ proto/api.proto