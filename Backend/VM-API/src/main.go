package main

import (
	"VM-API/src/WebSocket"
	"log"
	"net/http"
)

func main() {
	http.HandleFunc("/ws", WebSocket.WsHandler)
	log.Println("[INFO] Server running on :8881")
	log.Println("Version: 0.0.1")
	log.Fatal(http.ListenAndServe(":8881", nil))
}
