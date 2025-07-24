package RedisWrapper

import (
	"context"
	"fmt"
	"github.com/redis/go-redis/v9"
)

func (rc *RedisClient) initRedisClient(cfg Config) {
	ctx, cancel := context.WithCancel(context.Background())
	client := &RedisClient{
		mode:   cfg.Mode,
		topic:  cfg.Topic,
		ctx:    ctx,
		cancel: cancel,
	}
}
