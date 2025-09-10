package ApiCommonTypes

import (
	"crypto/ecdh"
	"encoding/json"

	"github.com/gorilla/websocket"
)

type PayloadSkeleton struct {
	PayloadType     string          `json:"type"`
	InternalPayload json.RawMessage `json:"internalpayload"`
	IV              string          `json:"iv,omitempty"`
	Token           string          `json:"token"`
}

type MessagePayload struct {
	Content      string `json:"content"`
	Timestamp    string `json:"timestamp"`
	SenderID     string `json:"senderID"`
	ReceiverType string `json:"messageType"`
	ReceiverID   string `json:"receiverID"`
	MessageID    string `json:"messageID"`
}

type WebSocketSession struct {
	Conn              *websocket.Conn
	ClientUUID        string
	SharedSecret      []byte
	APIHandshakeDone  bool
	AUTHHandshakeDone bool
	PrivKey           *ecdh.PrivateKey
	PubKey            *ecdh.PublicKey
}
