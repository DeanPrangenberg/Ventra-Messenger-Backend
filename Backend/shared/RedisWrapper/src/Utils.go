package src

import (
	"encoding/json"
	"fmt"
	"github.com/redis/go-redis/v9"
	"time"
)

func (rw *RedisWorker) EnsureRedisConnection() error {
	for {
		if rw.redisClient == nil {
			rw.redisClient = redis.NewClient(&redis.Options{
				Addr:     rw.redisAddr,
				Password: rw.redisPass,
				DB:       rw.redisDB,
			})
		}
		_, err := rw.redisClient.Ping(rw.ctx).Result()
		if err == nil {
			return nil
		}
		fmt.Println("Redis is not reachable, will try again in 5 sec:", err)
		time.Sleep(5 * time.Second)
		rw.redisClient = nil
	}
}

type Pkg struct {
	MsgType string          `json:"type"`
	Pkg     json.RawMessage `json:"pkgPipe"`
	IV      string          `json:"iv,omitempty"`
}

type MessagePkg struct {
	Content      string `json:"content"`
	Timestamp    string `json:"timestamp"`
	SenderID     string `json:"senderID"`
	ReceiverType string `json:"messageType"`
	ReceiverID   string `json:"receiverID"`
	MessageID    string `json:"messageID"`
}
