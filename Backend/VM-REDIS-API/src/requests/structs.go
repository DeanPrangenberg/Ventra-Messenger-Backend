package requests

import (
	gRPC "Redis-API-Wrapper/Redis-API-gRPC"
	"github.com/redis/go-redis/v9"
)

type Server struct {
	gRPC.UnimplementedUserStatusServiceServer
	RedisClient *redis.Client
}
