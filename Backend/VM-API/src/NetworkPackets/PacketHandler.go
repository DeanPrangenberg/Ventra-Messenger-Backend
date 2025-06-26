package NetworkPackets

import (
	"crypto/ecdh"
	"encoding/json"
	"github.com/gorilla/websocket"
	"log"
)

type WebSocketSession struct {
	Conn          *websocket.Conn
	ClientUUID    string
	SharedSecret  []byte
	HandShakeDone bool
	privKey       *ecdh.PrivateKey
	pubKey        *ecdh.PublicKey
}

func ProcessPkg(sessionInfo *WebSocketSession, data []byte) error {
	var pkg Pkg
	if err := json.Unmarshal(data, &pkg); err != nil {
		log.Printf("[ERROR] Failed to unmarshal package: %v", err)
		return err
	}

	switch pkg.MsgType {
	case "Handshake":
		return handleHandshake(sessionInfo, pkg.Pkg)
	case "MessagePkg":
		if !sessionInfo.HandShakeDone {
			log.Println("[WARN] Handshake not done, ignoring MessagePkg")
			return nil
		}
		return handleEncryptedMessage(sessionInfo, pkg)
	default:
		log.Printf("[WARN] Unknown package type: %s", pkg.MsgType)
		return nil
	}
}
