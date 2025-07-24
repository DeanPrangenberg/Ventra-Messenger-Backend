package MessageHandlers

import (
	"VM-API/src/commonTypes"
	"github.com/gorilla/websocket"
	"log"
)

func HandleMessage(session *commonTypes.WebSocketSession, message commonTypes.MessagePkg) error {
	switch message.ReceiverType {
	case "DM":
		log.Println("[INFO] Received DM message publishing to Redis")
		err := RedisWrapper.PubNewDMMessage(
			message.Content,
			message.Timestamp,
			message.SenderID,
			message.ReceiverID,
			message.MessageID,
		)
		if err != nil {
			session.Conn.WriteMessage(websocket.TextMessage, []byte("Failed to send message: "+err.Error()))
			return err
		}
		session.Conn.WriteMessage(websocket.TextMessage, []byte("Message sent successfully"))
		log.Println("[INFO] DM message published successfully")
	case "GROUP":

	case "SERVER":

	default:
		log.Fatal("[ERROR] Unknown receiver type: ", message.ReceiverType)
	}

	return nil
}
