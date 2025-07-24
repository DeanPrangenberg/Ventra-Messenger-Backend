package requests

import (
	"VM-REDIS-API/gRPC"
	gRPC2 "VM-REDIS-API/src/gRPC"
	"context"
	"fmt"
)

func (s *Server) SetUserSession(ctx context.Context, req *gRPC2.SetUserSessionRequest) (*gRPC2.StatusResponse, error) {
	err := s.RedisClient.Set(ctx, "user:session:"+req.UserId, req.Api, 0).Err()
	if err != nil {
		return nil, err
	}

	return &gRPC2.StatusResponse{
		Message: fmt.Sprint("Set UserSession", req.UserId, "to", req.Api),
		Success: true,
	}, nil
}

func (s *Server) GetUserSession(ctx context.Context, req *gRPC2.GetUserSessionRequest) (*gRPC2.GetUserSessionResponse, error) {
	api, err := s.RedisClient.Get(ctx, "user:session:"+req.UserId).Result()
	if err != nil {
		return nil, err
	}

	return &gRPC2.GetUserSessionResponse{
		Api: api,
	}, nil
}
