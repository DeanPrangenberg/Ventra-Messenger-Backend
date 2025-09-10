package PayloadUtils

import (
	"VM-API/src/ApiCommonTypes"
	"log"
)

func requireHandshake(session *ApiCommonTypes.WebSocketSession) bool {
	if !session.HandshakeDone {
		log.Println("[WARN] Handshake not done, ignoring payload")
		return false
	}
	return true
}
