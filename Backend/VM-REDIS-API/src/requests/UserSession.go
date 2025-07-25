package requests

import (
	gRPC "Redis-API-Wrapper/Redis-API-gRPC"
	"context"
	"fmt"
)

func (s *Server) SetUserSession(ctx context.Context, req *gRPC.SetUserSessionRequest) (*gRPC.StatusResponse, error) {
	err := s.RedisClient.Set(ctx, "user:session:"+req.UserId, req.Api, 0).Err()
	if err != nil {
		return nil, err
	}

	return &gRPC.StatusResponse{
		Message: fmt.Sprint("Set UserSession", req.UserId, "to", req.Api),
		Success: true,
	}, nil
}

func (s *Server) GetUserSession(ctx context.Context, req *gRPC.GetUserSessionRequest) (*gRPC.GetUserSessionResponse, error) {
	api, err := s.RedisClient.Get(ctx, "user:session:"+req.UserId).Result()
	if err != nil {
		return nil, err
	}

	return &gRPC.GetUserSessionResponse{
		Api: api,
	}, nil
}
