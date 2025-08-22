package WebSocket

import (
	"VM-API/src/PayloadHandlers"
	"VM-API/src/commonTypes"
	"log"
	"net/http"
	"sync"

	"github.com/google/uuid"
	"github.com/gorilla/websocket"
)

var (
	connections = make(map[string]*websocket.Conn)
	connMutex   sync.Mutex
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
	id := uuid.New().String()
	log.Printf("[INFO] Client connected: %s with UUID: %s", r.RemoteAddr, id)

	go handleClient(conn, r.RemoteAddr, id)
}

func handleClient(conn *websocket.Conn, remoteAddr string, uuid string) {
	defer conn.Close()
	defer func() {
		connMutex.Lock()
		delete(connections, uuid)
		connMutex.Unlock()
	}()
	session := &commonTypes.WebSocketSession{Conn: conn, ClientUUID: uuid, HandShakeDone: false}

	connMutex.Lock()
	connections[uuid] = conn
	connMutex.Unlock()

	for {
		mt, msg, err := conn.ReadMessage()
		if err != nil {
			log.Printf("[ERROR] Read error from %s: %v", remoteAddr, err)
			break
		}

		if err := PayloadHandlers.ProcessPkg(session, msg); err != nil {
			log.Printf("[ERROR] Processing package failed for %s: %v", remoteAddr, err)
			break
		}

		if err := conn.WriteMessage(mt, msg); err != nil {
			log.Printf("[ERROR] Write error to %s: %v", remoteAddr, err)
			break
		}
	}
}
