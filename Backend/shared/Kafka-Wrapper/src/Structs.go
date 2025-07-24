package KafkaWrapper

import (
	"context"
	"github.com/segmentio/kafka-go"
	"sync"
)

type workerPool struct {
	count    int
	min      int
	max      int
	mu       sync.Mutex
	stops    []chan struct{}
	handler  func(interface{})
	pipeType string
}

type KafkaClient struct {
	reader     *kafka.Reader
	writer     *kafka.Writer
	mode       string
	topic      string
	ctx        context.Context
	cancel     context.CancelFunc
	wg         sync.WaitGroup
	OutPipe    chan kafka.Message
	InPipe     chan []byte
	consumerWP *workerPool
	producerWP *workerPool
}

type Config struct {
	Brokers            []string
	Topic              string
	GroupID            string
	Mode               string // "consumer", "producer", or "both"
	PipeBuffer         int
	ConsumerMinWorkers int
	ConsumerMaxWorkers int
	ProducerMinWorkers int
	ProducerMaxWorkers int
	ConsumerHandler    func(interface{})
	ProducerHandler    func(interface{})
}
