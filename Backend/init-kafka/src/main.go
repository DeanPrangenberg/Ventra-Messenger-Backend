package main

import (
	"fmt"
	"log"
	"time"

	"github.com/segmentio/kafka-go"
)

var (
	brokerAddress = "kafka:9092"
	topics        = []string{
		"messages",
		"user-status",
		"user-auth-event",
		"stats-updates",
		"key-rotation",
	}
)

func main() {
	// Wait for Kafka
	for {
		conn, err := kafka.Dial("tcp", brokerAddress)
		if err != nil {
			log.Println("Waiting for Kafka...", err)
			time.Sleep(3 * time.Second)
			continue
		}
		_ = conn.Close()
		break
	}

	// Create topics
	conn, err := kafka.Dial("tcp", brokerAddress)
	if err != nil {
		log.Fatal("Kafka not reachable:", err)
	}
	defer conn.Close()

	controller, err := conn.Controller()
	if err != nil {
		log.Fatal("Could not find Kafka controller:", err)
	}
	conn.Close()

	controllerConn, err := kafka.Dial("tcp", fmt.Sprintf("%s:%d", controller.Host, controller.Port))
	if err != nil {
		log.Fatal("Failed to connect to Kafka controller:", err)
	}
	defer controllerConn.Close()

	for _, topic := range topics {
		if topic == "" {
			continue
		}
		topicConfig := kafka.TopicConfig{
			Topic:             topic,
			NumPartitions:     1,
			ReplicationFactor: 1,
		}
		err := controllerConn.CreateTopics(topicConfig)
		if err != nil {
			log.Printf("Could not create topic %s: %s\n", topic, err)
		} else {
			log.Printf("Topic %s created\n", topic)
		}
	}
}
