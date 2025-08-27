package main

import (
	"VM-API/src/PrometheusEndpoint"
	"VM-API/src/WebSocket"
	gRPCserver "VM-API/src/gRPC-Server"
	"log"
	"net/http"
)

func main() {
	// Start Prometheus endpoint
	go PrometheusEndpoint.StartPrometheusEndpoint()
	go gRPCserver.StartGRPCServer()

	go func() {
		for i := 0; i < 10000; i++ {
			if i != 10000 {
				PrometheusEndpoint.TestCounter.Inc()
			} else {
				PrometheusEndpoint.TestCounter.Desc()
			}
			if i > 5000 {
				PrometheusEndpoint.TestGauge.Dec()
			} else {
				PrometheusEndpoint.TestGauge.Inc()
			}
		}
	}()

	// Set up WebSocket handler
	http.HandleFunc("/ws", WebSocket.WsHandler)
	log.Println("[INFO] VM-API Server running on  Port: 8881")
	log.Println("Version: 0.0.3")
	log.Fatal(http.ListenAndServe(":8881", nil))
}
