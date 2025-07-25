package RedisWrapper

import (
	Redis_gRPC "Redis-API-Wrapper/Redis-API-gRPC"
	"context"
	"google.golang.org/grpc"
)

type Client struct {
	conn        *grpc.ClientConn
	Client      Redis_gRPC.UserStatusServiceClient
	Ctx         context.Context
	DebugPrints bool
}
