package KafkaWrapper

import (
	"errors"
	"github.com/segmentio/kafka-go"
)

// ReadMessage reads a message from Kafka (consumer mode)
func (kc *KafkaClient) ReadMessage() (kafka.Message, error) {
	if kc.reader == nil {
		return kafka.Message{}, errors.New("KafkaClient not in consumer mode")
	}
	return kc.reader.ReadMessage(kc.ctx)
}
