package requests

import (
	"VM-REDIS-API/gRPC"
	gRPC2 "VM-REDIS-API/src/gRPC"
	"context"
	"fmt"
)

func (s *Server) SetUserStatus(ctx context.Context, req *gRPC2.UserStatusRequest) (*gRPC2.StatusResponse, error) {
	err := s.RedisClient.Set(ctx, "user:status:"+req.UserId, req.Status, 0).Err()
	if err != nil {
		return nil, err
	}

	return &gRPC2.StatusResponse{
		Message: fmt.Sprint("Set UserStatus", req.UserId, "to", req.Status),
		Success: true,
	}, nil
}

func (s *Server) GetUserStatus(ctx context.Context, req *gRPC2.UserID) (*gRPC2.UserStatusResponse, error) {
	status, err := s.RedisClient.Get(ctx, "user:status:"+req.UserId).Result()
	if err != nil {
		return nil, err
	}

	return &gRPC2.UserStatusResponse{
		Status: status,
	}, nil
}
