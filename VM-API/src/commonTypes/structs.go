package commonTypes

import (
	"crypto/ecdh"
	"encoding/json"
	"github.com/gorilla/websocket"
)

type Pkg struct {
	MsgType string          `json:"type"`
	Pkg     json.RawMessage `json:"pkg"`
	IV      string          `json:"iv,omitempty"`
}

type MessagePkg struct {
	Content      string `json:"content"`
	Timestamp    string `json:"timestamp"`
	SenderID     string `json:"senderID"`
	ReceiverType string `json:"messageType"`
	ReceiverID   string `json:"receiverID"`
	MessageID    string `json:"messageID"`
}

type WebSocketSession struct {
	Conn          *websocket.Conn
	ClientUUID    string
	SharedSecret  []byte
	HandShakeDone bool
	PrivKey       *ecdh.PrivateKey
	PubKey        *ecdh.PublicKey
}
