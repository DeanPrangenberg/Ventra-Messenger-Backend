package KafkaWrapper

import (
	"errors"
	"github.com/segmentio/kafka-go"
)

// SendMessage sends a message to Kafka (producer mode)
func (kc *KafkaClient) SendMessage(value []byte) error {
	if kc.writer == nil {
		return errors.New("KafkaClient not in producer mode")
	}
	return kc.writer.WriteMessages(kc.ctx, kafka.Message{Value: value})
}
