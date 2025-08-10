package requests

import (
	gRPC "Redis-API-Wrapper/Redis-API-gRPC"
	"context"
	"fmt"
)

func (s *Server) SetUserStatus(ctx context.Context, req *gRPC.UserStatusRequest) (*gRPC.StatusResponse, error) {
	err := s.RedisClient.Set(ctx, "user:status:"+req.UserId, req.Status, 0).Err()
	if err != nil {
		return nil, err
	}

	return &gRPC.StatusResponse{
		Message: fmt.Sprint("Set UserStatus", req.UserId, "to", req.Status),
		Success: true,
	}, nil
}

func (s *Server) GetUserStatus(ctx context.Context, req *gRPC.UserID) (*gRPC.UserStatusResponse, error) {
	status, err := s.RedisClient.Get(ctx, "user:status:"+req.UserId).Result()
	if err != nil {
		return nil, err
	}

	return &gRPC.UserStatusResponse{
		Status: status,
	}, nil
}
