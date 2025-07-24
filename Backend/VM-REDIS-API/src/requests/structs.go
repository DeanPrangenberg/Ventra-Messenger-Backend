package requests

import (
	"VM-REDIS-API/src/gRPC"
	"github.com/redis/go-redis/v9"
)

type Server struct {
	gRPC.UnimplementedUserStatusServiceServer
	RedisClient *redis.Client
}
