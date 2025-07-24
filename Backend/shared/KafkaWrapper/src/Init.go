package src

import (
	"context"
	"github.com/segmentio/kafka-go"
)

// NewKafkaClient Initializes a new KafkaClient with the given configuration.
// Sets up pipes and background goroutines for reading/writing.
func NewKafkaClient(cfg Config) *KafkaClient {
	ctx, cancel := context.WithCancel(context.Background())
	client := &KafkaClient{
		mode:   cfg.Mode,
		topic:  cfg.Topic,
		ctx:    ctx,
		cancel: cancel,
	}
	// Consumer setup
	if cfg.Mode == "consumer" || cfg.Mode == "both" {
		client.reader = kafka.NewReader(kafka.ReaderConfig{
			Brokers: cfg.Brokers,
			Topic:   cfg.Topic,
			GroupID: cfg.GroupID,
		})
		buffer := 100
		if cfg.PipeBuffer > 0 {
			buffer = cfg.PipeBuffer
		}
		client.OutPipe = make(chan kafka.Message, buffer)
		go func() {
			for {
				msg, err := client.reader.ReadMessage(client.ctx)
				if err != nil {
					if client.ctx.Err() != nil {
						close(client.OutPipe)
						return
					}
					continue
				}
				client.OutPipe <- msg
			}
		}()
		// Start consumer worker pool if handler is set
		if cfg.ConsumerHandler != nil && cfg.ConsumerMinWorkers > 0 && cfg.ConsumerMaxWorkers > 0 {
			client.StartWorkerPool("consumer", cfg.ConsumerMinWorkers, cfg.ConsumerMaxWorkers, cfg.ConsumerHandler)
		}
	}
	// Producer setup
	if cfg.Mode == "producer" || cfg.Mode == "both" {
		client.writer = &kafka.Writer{
			Addr:  kafka.TCP(cfg.Brokers...),
			Topic: cfg.Topic,
		}
		buffer := 100
		if cfg.PipeBuffer > 0 {
			buffer = cfg.PipeBuffer
		}
		client.InPipe = make(chan []byte, buffer)
		go func() {
			for {
				select {
				case <-client.ctx.Done():
					return
				case msg := <-client.InPipe:
					client.writer.WriteMessages(client.ctx, kafka.Message{Value: msg})
				}
			}
		}()
		// Start producer worker pool if handler is set
		if cfg.ProducerHandler != nil && cfg.ProducerMinWorkers > 0 && cfg.ProducerMaxWorkers > 0 {
			client.StartWorkerPool("producer", cfg.ProducerMinWorkers, cfg.ProducerMaxWorkers, cfg.ProducerHandler)
		}
	}
	return client
}

// Starts a worker pool for the given pipe type ("consumer" or "producer").
// Each worker will call the handler function for messages from the pipe.
func (kc *KafkaClient) StartWorkerPool(pipeType string, min, max int, handler func(interface{})) {
	wp := &workerPool{
		count:    min,
		min:      min,
		max:      max,
		stops:    make([]chan struct{}, 0, max),
		handler:  handler,
		pipeType: pipeType,
	}
	for i := 0; i < min; i++ {
		kc.startWorker(wp)
	}
	if pipeType == "consumer" {
		kc.consumerWP = wp
	} else if pipeType == "producer" {
		kc.producerWP = wp
	}
}
