package PayloadUtils

import (
	"VM-API/src/ApiCommonTypes"
	"encoding/json"
	"log"

	"github.com/gorilla/websocket"
)

func SendEncryptedResponse(session *ApiCommonTypes.WebSocketSession, requestID, responseType string, payload interface{}) error {
	var payloadBytes []byte
	var err error

	switch v := payload.(type) {
	case []byte:
		payloadBytes = v
	default:
		payloadBytes, err = json.Marshal(payload)
		if err != nil {
			return err
		}
	}

	encPayload, err := EncryptInternalPayload(session, requestID, responseType, payloadBytes)
	if err != nil {
		return err
	}

	return session.Conn.WriteMessage(websocket.TextMessage, encPayload)
}

func SendErrorResponse(session *ApiCommonTypes.WebSocketSession, requestID, errorMsg string) error {
	log.Println(errorMsg)

	errorPayload := ApiCommonTypes.ErrorPayload{
		Error: errorMsg,
	}

	return SendEncryptedResponse(session, requestID, "error", errorPayload)
}
