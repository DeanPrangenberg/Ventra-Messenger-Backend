package Redis

import (
	"VM-API/src/commonTypes"
	"encoding/json"
	"fmt"
	"sync"
	"time"
)

type MessageWriter struct {
	jobQueue   chan commonTypes.Pkg
	workers    []*redisAPI.RedisWorker
	mu         sync.Mutex
	maxWorkers int
	minWorkers int
	active     int
	stopChans  []chan struct{}
}

func NewMessageWriter(minWorkers, maxWorkers, queueSize int) *MessageWriter {
	mw := &MessageWriter{
		jobQueue:   make(chan commonTypes.Pkg, queueSize),
		workers:    make([]*redisAPI.RedisWorker, 0, maxWorkers),
		maxWorkers: maxWorkers,
		minWorkers: minWorkers,
		active:     0,
		stopChans:  make([]chan struct{}, 0, maxWorkers),
	}
	for i := 0; i < minWorkers; i++ {
		mw.addWorker()
	}
	go mw.scaleWorkers()
	return mw
}

func (mw *MessageWriter) addWorker() {
	mw.mu.Lock()
	defer mw.mu.Unlock()
	rw := redisAPI.CreateRedisWorker()
	stop := make(chan struct{})
	mw.workers = append(mw.workers, rw)
	mw.stopChans = append(mw.stopChans, stop)
	mw.active++
	go mw.startWorker(rw, stop)
}

func (mw *MessageWriter) removeWorker() {
	mw.mu.Lock()
	defer mw.mu.Unlock()
	if mw.active > mw.minWorkers {
		stop := mw.stopChans[mw.active-1]
		close(stop)
		mw.stopChans = mw.stopChans[:mw.active-1]
		mw.workers = mw.workers[:mw.active-1]
		mw.active--
	}
}

func (mw *MessageWriter) startWorker(rw *redisAPI.RedisWorker, stop <-chan struct{}) {
	for {
		select {
		case p := <-mw.jobQueue:
			mw.handleMessage(rw, p)
		case <-stop:
			return
		}
	}
}

func (mw *MessageWriter) handleMessage(rw *redisAPI.RedisWorker, p commonTypes.Pkg) {
	switch p.MsgType {
	case "dm:new":
		var msg commonTypes.MessagePkg
		if err := json.Unmarshal(p.Pkg, &msg); err != nil {
			fmt.Println("Error unmarshalling message:", err)
			return
		}
		if err := rw.PubNewDMMessage(msg.Content, msg.Timestamp, msg.SenderID, msg.ReceiverID, msg.MessageID); err != nil {
			fmt.Println("Error publishing new DM message:", err)
		}
	default:
		fmt.Println("Unknown message type:", p.MsgType)
	}
}

func (mw *MessageWriter) scaleWorkers() {
	ticker := time.NewTicker(2 * time.Second)
	defer ticker.Stop()
	for range ticker.C {
		queueLen := len(mw.jobQueue)
		mw.mu.Lock()
		active := mw.active
		mw.mu.Unlock()
		if queueLen > active && active < mw.maxWorkers {
			mw.addWorker()
		} else if queueLen < active-1 && active > mw.minWorkers {
			mw.removeWorker()
		}
	}
}

func (mw *MessageWriter) DispatchMessages(pkg <-chan commonTypes.Pkg) {
	for p := range pkg {
		mw.jobQueue <- p
	}
}
