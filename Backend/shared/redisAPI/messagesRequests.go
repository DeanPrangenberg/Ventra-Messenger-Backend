package redisAPI

import (
	"context"
	"fmt"
	"github.com/redis/go-redis/v9"
	"time"
)

var (
	ctx         = context.Background()
	redisClient *redis.Client
	redisAddr   = "vm-redis:6379"
	redisPass   = ""
	redisDB     = 0
)

// Verbindungsaufbau mit Retry-Mechanismus
func ensureRedisConnection() error {
	for {
		if redisClient == nil {
			redisClient = redis.NewClient(&redis.Options{
				Addr:     redisAddr,
				Password: redisPass,
				DB:       redisDB,
			})
		}
		_, err := redisClient.Ping(ctx).Result()
		if err == nil {
			return nil
		}
		fmt.Println("Redis nicht erreichbar, neuer Versuch in 5 Sekunden:", err)
		time.Sleep(5 * time.Second)
		redisClient = nil // Neu initialisieren beim nächsten Versuch
	}
}

// Wrapper für Redis-Operationen
func withRedis(f func() error) error {
	if err := ensureRedisConnection(); err != nil {
		return err
	}
	return f()
}

func PubNewDMMessage(Content string, Timestamp string, SenderID string, ReceiverID string, MessageID string) error {
	return withRedis(func() error {
		channel := "dm:new"
		message := fmt.Sprintf("%s|%s|%s|%s|%s", MessageID, Content, Timestamp, SenderID, ReceiverID)
		return redisClient.Publish(ctx, channel, message).Err()
	})
}
