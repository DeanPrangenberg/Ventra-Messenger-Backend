package main

import (
	gRPC "Redis-API-Wrapper/Redis-API-gRPC"
	"VM-REDIS-API/src/requests"
	"github.com/redis/go-redis/v9"
	"log"
	"net"

	"google.golang.org/grpc"
)

func main() {
	lis, err := net.Listen("tcp", ":8886")
	if err != nil {
		log.Fatalf("failed to listen: %v", err)
	}

	grpcServer := grpc.NewServer()
	gRPC.RegisterUserStatusServiceServer(grpcServer, &requests.Server{
		gRPC.UnimplementedUserStatusServiceServer{},
		redis.NewClient(
			&redis.Options{
				Addr: "localhost:6379",
			}),
	})

	log.Println("Server running on :8891")
	if err := grpcServer.Serve(lis); err != nil {
		log.Fatalf("failed to serve: %v", err)
	}
}
