package pkges

import "encoding/json"

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
