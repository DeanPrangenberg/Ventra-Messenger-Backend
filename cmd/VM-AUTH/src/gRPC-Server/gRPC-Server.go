package gRPCserver

import (
	pb "VM-AUTH-gRPC-Wrapper/gen-pb"
	"VM-AUTH/src/JWT-Tokens"
	"log"
	"net"

	"google.golang.org/grpc"
)

type VMAuthServer struct {
	pb.UnimplementedUserAuthServer
	JM *JWT_Tokens.TokenManager
}

func StartGRPCServer() {
	lis, err := net.Listen("tcp", ":4445")
	if err != nil {
		log.Fatalf("failed to listen: %v", err)
	}

	s := grpc.NewServer()

	log.Println("gRPC server listening on :4445")

	if err := s.Serve(lis); err != nil {
		log.Fatalf("failed to serve: %v", err)
	}
}
