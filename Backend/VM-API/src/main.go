package main

import (
	"VM-API/src/WebSocket"
	"VM-API/src/commonTypes"
	"VM-API/src/workers/Redis"
	"log"
	"net/http"
)

type WorkerPool struct {
	redisWriterWorker *Redis.MessageWriter
	redisWriterQueue  chan commonTypes.Pkg
}

func setupWorkerPools() *WorkerPool {
	return &WorkerPool{
		redisWriterWorker: Redis.NewMessageWriter(1, 50, 1000),
		redisWriterQueue:  make(chan commonTypes.Pkg, 1000),
	}
}

func main() {
	workerPool := setupWorkerPools()

	go WorkerPool.writerWorker.DispatchMessages(WorkerPool.writerQueue)

	http.HandleFunc("/ws", WebSocket.WsHandler)
	log.Println("[INFO] VM-API Server running on  Port: 8881")
	log.Println("Version: 0.0.2")
	log.Fatal(http.ListenAndServe(":8881", nil))
}
