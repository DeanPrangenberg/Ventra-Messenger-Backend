package main

import (
	"VM-API/src/PrometheusEndpoint"
	"VM-API/src/WebSocket"
	"log"
	"net/http"
)

func main() {
	// Start Prometheus endpoint
	go PrometheusEndpoint.StartPrometheusEndpoint()

	// Set up WebSocket handler
	http.HandleFunc("/ws", WebSocket.WsHandler)
	log.Println("[INFO] VM-API Server running on  Port: 8881")
	log.Println("Version: 0.0.3")
	log.Fatal(http.ListenAndServe(":8881", nil))
}
