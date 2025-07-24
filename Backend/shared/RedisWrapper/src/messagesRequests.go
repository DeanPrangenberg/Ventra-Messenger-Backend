package src

import (
	"fmt"
)

func (rw *RedisWorker) PubNewDMMessage(Content string, Timestamp string, SenderID string, ReceiverID string, MessageID string) error {
	channel := "dm:new"
	message := fmt.Sprintf("%s|%s|%s|%s|%s", MessageID, Content, Timestamp, SenderID, ReceiverID)
	return rw.redisClient.Publish(rw.ctx, channel, message).Err()
}
