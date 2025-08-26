package RedisWrapper

import (
	"context"

	"github.com/redis/go-redis/v9"
)

type Client struct {
	RedisClient *redis.Client
	Ctx         context.Context
	DebugPrints bool
}
