package gRPCserver

import (
	"VM-API/src/ConnectionManager"
	"context"
	"log"

	pb "VM-API-gRPC-Wrapper/gen-pb"
)

func (s *server) IsUserConnected(ctx context.Context, req *pb.UserStatusRequest) (*pb.UserStatusResponse, error) {
	log.Printf("Checking connection status for user: %s", req.UserId)
	connected := ConnectionManager.ConnectionExists(req.UserId)
	return &pb.UserStatusResponse{Connected: connected}, nil
}

func (s *server) SendPayload(ctx context.Context, req *pb.PayloadRequest) (*pb.PayloadResponse, error) {
	log.Printf("Sending payload to user: %s", req.UserId)
	conn, ok := ConnectionManager.GetConnection(req.UserId)
	if ok == false {
		log.Printf("No active connection for user: %s", req.UserId)
		return &pb.PayloadResponse{Ack: "No active connection"}, nil
	}

	if err := conn.WriteMessage(1, req.Payload); err != nil {
		log.Printf("Failed to send payload to user %s: %v", req.UserId, err)
		return &pb.PayloadResponse{Ack: "Failed to send payload"}, err
	}
	return &pb.PayloadResponse{Ack: "Payload send"}, nil
}
