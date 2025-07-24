package RedisWrapper

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

type RedisClient struct {
	reader     *kafka.Reader
	writer     *kafka.Writer
	ctx        context.Context
	cancel     context.CancelFunc
	mode       string // read, write, or both
	wg         sync.WaitGroup
	OutPipe    chan kafka.Message
	InPipe     chan []byte
	consumerWP *workerPool
	producerWP *workerPool
}

type Config struct {
	Address          string
	Port             int
	Password         string
	Database         int
	Mode             string // read, write, or both
	PipeBuffer       int
	ReaderMinWorkers int
	ReaderMaxWorkers int
	WriterMinWorkers int
	WriterMaxWorkers int
	ReaderHandler    func(interface{})
	WriterHandler    func(interface{})
}
