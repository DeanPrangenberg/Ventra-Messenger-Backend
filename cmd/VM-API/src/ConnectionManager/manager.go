package ConnectionManager

import (
	"VM-API/src/PrometheusEndpoint"
	"sync"

	"github.com/gorilla/websocket"
	"log"
)

var (
	connections = make(map[string]*websocket.Conn)
	connMutex   sync.Mutex
)

func AddConnection(uuid string, conn *websocket.Conn) {
	connMutex.Lock()
	defer connMutex.Unlock()
	connections[uuid] = conn
	PrometheusEndpoint.ConnectedClients.Inc()
	log.Println(uuid, " added. Total connections:", len(connections))
}

func RemoveConnection(uuid string) {
	connMutex.Lock()
	defer connMutex.Unlock()
	delete(connections, uuid)
	PrometheusEndpoint.ConnectedClients.Dec()
	log.Println(uuid, " removed. Total connections:", len(connections))
}

func ConnectionExists(uuid string) bool {
	connMutex.Lock()
	defer connMutex.Unlock()
	_, exists := connections[uuid]
	return exists
}

func GetConnection(uuid string) (*websocket.Conn, bool) {
	connMutex.Lock()
	defer connMutex.Unlock()
	conn, exists := connections[uuid]
	return conn, exists
}

func GetAllConnections() map[string]*websocket.Conn {
	connMutex.Lock()
	defer connMutex.Unlock()
	// Kopie zurückgeben, um Race Conditions zu vermeiden
	copy := make(map[string]*websocket.Conn)
	for k, v := range connections {
		copy[k] = v
	}
	return copy
}
