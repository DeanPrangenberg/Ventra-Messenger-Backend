package src

// Graceful shutdown
func (kc *KafkaClient) Shutdown() {
	kc.cancel()
	// Stop consumer workers
	if kc.consumerWP != nil {
		kc.consumerWP.mu.Lock()
		for _, stop := range kc.consumerWP.stops {
			close(stop)
		}
		kc.consumerWP.stops = nil
		kc.consumerWP.mu.Unlock()
	}
	// Stop producer workers
	if kc.producerWP != nil {
		kc.producerWP.mu.Lock()
		for _, stop := range kc.producerWP.stops {
			close(stop)
		}
		kc.producerWP.stops = nil
		kc.producerWP.mu.Unlock()
	}
	kc.wg.Wait()
	if kc.reader != nil {
		kc.reader.Close()
	}
	if kc.writer != nil {
		kc.writer.Close()
	}
}

func (kc *KafkaClient) ResizeWorkers(n int, pipeType string) {
	var wp *workerPool
	if pipeType == "consumer" {
		wp = kc.consumerWP
	} else if pipeType == "producer" {
		wp = kc.producerWP
	} else {
		return
	}
	wp.mu.Lock()
	defer wp.mu.Unlock()
	if n > wp.max {
		n = wp.max
	}
	if n < wp.min {
		n = wp.min
	}
	diff := n - wp.count
	if diff > 0 {
		for i := 0; i < diff; i++ {
			kc.startWorker(wp)
		}
	} else if diff < 0 {
		for i := 0; i < -diff; i++ {
			close(wp.stops[len(wp.stops)-1])
			wp.stops = wp.stops[:len(wp.stops)-1]
		}
	}
	wp.count = n
}

func (kc *KafkaClient) startWorker(wp *workerPool) {
	stop := make(chan struct{})
	wp.stops = append(wp.stops, stop)
	kc.wg.Add(1)
	go func() {
		defer kc.wg.Done()
		for {
			select {
			case <-stop:
				return
			default:
				if wp.pipeType == "consumer" && kc.OutPipe != nil {
					select {
					case msg, ok := <-kc.OutPipe:
						if !ok {
							return
						}
						wp.handler(msg)
					case <-kc.ctx.Done():
						return
					}
				} else if wp.pipeType == "producer" && kc.InPipe != nil {
					select {
					case msg, ok := <-kc.InPipe:
						if !ok {
							return
						}
						wp.handler(msg)
					case <-kc.ctx.Done():
						return
					}
				}
			}
		}
	}()
}
