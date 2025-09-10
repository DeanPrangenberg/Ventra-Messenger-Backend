package ConnectionManager

import (
	"VM-API/src/PrometheusEndpoint"
	VM_AUTH_gRPC_Wrapper "VM-AUTH-gRPC-Wrapper/src"
	"sync"

	"log"

	"github.com/gorilla/websocket"
)

var (
	connections     = make(map[*string]*websocket.Conn)
	connMutex       sync.Mutex
	AuthClient      *VM_AUTH_gRPC_Wrapper.VmAuthClient
	AuthClientMutex sync.Mutex
)

func AddConnection(uuid *string, conn *websocket.Conn) {
	connMutex.Lock()
	defer connMutex.Unlock()
	connections[uuid] = conn
	PrometheusEndpoint.ConnectedClients.Inc()
	log.Println(uuid, " added. Total connections:", len(connections))
}

func RemoveConnection(uuid *string) {
	connMutex.Lock()
	defer connMutex.Unlock()
	delete(connections, uuid)
	PrometheusEndpoint.ConnectedClients.Dec()
	log.Println(uuid, " removed. Total connections:", len(connections))
}

func ConnectionExists(uuid string) bool {
	connMutex.Lock()
	defer connMutex.Unlock()
	_, exists := connections[&uuid]
	return exists
}

func GetConnection(uuid string) (*websocket.Conn, bool) {
	connMutex.Lock()
	defer connMutex.Unlock()
	conn, exists := connections[&uuid]
	return conn, exists
}

func GetAuthClient() *VM_AUTH_gRPC_Wrapper.VmAuthClient {
	AuthClientMutex.Lock()
	defer AuthClientMutex.Unlock()

	if AuthClient != nil {
		return AuthClient
	}

	var err error
	AuthClient, err = VM_AUTH_gRPC_Wrapper.NewVmAuthClient("vm-auth:4445")
	if err != nil {
		log.Fatalf("Failed to create Auth gRPC client: %v", err)
	}
	log.Println("Auth gRPC client created")
	return AuthClient
}
