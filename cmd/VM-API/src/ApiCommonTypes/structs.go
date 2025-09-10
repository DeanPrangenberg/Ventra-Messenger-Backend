package ApiCommonTypes

import (
	"crypto/ecdh"
	"encoding/json"

	"github.com/gorilla/websocket"
)

type PayloadSkeleton struct {
	RequestID       string          `json:"requestID"`
	PayloadType     string          `json:"payloadType"`
	InternalPayload json.RawMessage `json:"internalPayload"`
	IV              string          `json:"iv,omitempty"`
	Token           string          `json:"token,omitempty"`
	UserID          string          `json:"userID,omitempty"`
}

type ErrorPayload struct {
	Error string `json:"error"`
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
	Conn          *websocket.Conn
	ClientUUID    string
	SharedSecret  []byte
	HandshakeDone bool
	PrivKey       *ecdh.PrivateKey
	PubKey        *ecdh.PublicKey
}
