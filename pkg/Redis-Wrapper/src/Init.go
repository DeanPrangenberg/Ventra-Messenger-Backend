package RedisWrapper

import (
	"context"
	"fmt"

	"github.com/redis/go-redis/v9"
)

func New(address string, debugPrints bool) (*Client, error) {
	client := redis.NewClient(&redis.Options{
		Addr: address,
	})

	ctx := context.Background()
	_, err := client.Ping(ctx).Result()
	if err != nil {
		return nil, fmt.Errorf("failed to connect to Redis: %w", err)
	}

	return &Client{
		RedisClient: client,
		Ctx:         ctx,
		DebugPrints: debugPrints,
	}, nil
}

func (c *Client) Close() error {
	return c.RedisClient.Close()
}
