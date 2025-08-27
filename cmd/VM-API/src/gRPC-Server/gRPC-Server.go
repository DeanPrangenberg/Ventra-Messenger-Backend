package gRPCserver

import (
	pb "VM-API-gRPC-Wrapper/gen-pb"
	"log"
	"net"

	"google.golang.org/grpc"
)

type server struct {
	pb.UnimplementedUserApiServer
}

func StartGRPCServer() {
	lis, err := net.Listen("tcp", ":4445")
	if err != nil {
		log.Fatalf("failed to listen: %v", err)
	}

	s := grpc.NewServer()

	pb.RegisterUserApiServer(s, &server{})
	log.Println("gRPC server listening on :4445")

	if err := s.Serve(lis); err != nil {
		log.Fatalf("failed to serve: %v", err)
	}
}
