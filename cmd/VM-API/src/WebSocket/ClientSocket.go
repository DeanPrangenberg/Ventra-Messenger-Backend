package WebSocket

import (
	"VM-API/src/ApiCommonTypes"
	"VM-API/src/ConnectionManager"
	"VM-API/src/Payloads/Handler"
	"VM-API/src/PrometheusEndpoint"
	"log"
	"net/http"

	"github.com/gorilla/websocket"
)

var upgrader = websocket.Upgrader{
	CheckOrigin: func(r *http.Request) bool { return true },
}

func WsHandler(w http.ResponseWriter, r *http.Request) {
	conn, err := upgrader.Upgrade(w, r, nil)
	if err != nil {
		log.Printf("[ERROR] WebSocket upgrade failed: %v", err)
		return
	}

	log.Printf("[INFO] Client connected: %s", r.RemoteAddr)

	go handleClient(conn, r.RemoteAddr)
}

func handleClient(conn *websocket.Conn, remoteAddr string) {
	defer conn.Close()
	session := &ApiCommonTypes.WebSocketSession{Conn: conn, HandshakeDone: false}
	defer ConnectionManager.RemoveConnection(&session.ClientUUID)

	ConnectionManager.AddConnection(&session.ClientUUID, conn)

	for {
		mt, msg, err := conn.ReadMessage()
		if err != nil {
			log.Printf("[ERROR] Read error from %s: %v", remoteAddr, err)
			break
		} else {
			PrometheusEndpoint.PayloadsReceived.Inc() // Increment after receiving
		}

		if err := Handler.DispatchPayload(session, msg); err != nil {
			log.Printf("[ERROR] Processing package failed for %s: %v", remoteAddr, err)
			PrometheusEndpoint.PayloadsProcessedFailed.Inc() // Increment on failure
			break
		} else {
			PrometheusEndpoint.PayloadsProcessedSuccessfully.Inc() // Increment on success
		}

		if err := conn.WriteMessage(mt, msg); err != nil {
			log.Printf("[ERROR] Write error to %s: %v", remoteAddr, err)
			break
		} else {
			PrometheusEndpoint.PayloadsSendToClient.Inc() // Increment after sending
		}
	}
}
