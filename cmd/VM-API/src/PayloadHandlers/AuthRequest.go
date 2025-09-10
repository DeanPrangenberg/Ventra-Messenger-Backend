package PayloadHandlers

import (
	_ "CryptoLib/src"
	"VM-API/src/ApiCommonTypes"
	_ "crypto/ecdh"
	_ "encoding/base64"
	_ "encoding/json"
	_ "log"

	_ "github.com/gorilla/websocket"
)

func handleAuthHandshakeRequest(session *ApiCommonTypes.WebSocketSession) error {

	return nil
}

func handleAuthPayload(session *ApiCommonTypes.WebSocketSession, internalPayload ApiCommonTypes.PayloadSkeleton) error {

	return nil
}
