package src

import (
	"context"
	"github.com/redis/go-redis/v9"
)

type RedisWorker struct {
	ctx         context.Context
	redisClient *redis.Client
	redisAddr   string
	redisPass   string
	redisDB     int
}

func CreateRedisWorker() *RedisWorker {
	rw := &RedisWorker{
		ctx:       context.Background(),
		redisAddr: "vm-redis:6379",
		redisPass: "",
		redisDB:   0,
	}
	rw.redisClient = redis.NewClient(&redis.Options{
		Addr:     rw.redisAddr,
		Password: rw.redisPass,
		DB:       rw.redisDB,
	})
	return rw
}
