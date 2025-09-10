package Handler

import (
	"VM-API/src/ApiCommonTypes"
	"VM-API/src/Payloads/PayloadUtils"
	"encoding/json"
	"fmt"
	"log"

	"github.com/gorilla/websocket"
)

// Handler function type now matches HandshakeHandler
type PayloadHandler func(*ApiCommonTypes.WebSocketSession, ApiCommonTypes.PayloadSkeleton) error

// Handler-Map
var payloadHandlers = map[string]PayloadHandler{
	"HandshakeRequest":      HandshakeHandler,
	"RegisterRequest":       RegisterHandler,
	"LoginRequest":          LoginHandler,
	"UpdatePasswordRequest": UpdatePasswordHandler,
	"UpdateUsernameRequest": UpdateUsernameHandler,
	"DeleteAccountRequest":  DeleteAccountHandler,
	"NewSessionRequest":     NewSessionHandler,
}

// Dispatcher
func DispatchPayload(sessionInfo *ApiCommonTypes.WebSocketSession, data []byte) error {
	var payload ApiCommonTypes.PayloadSkeleton
	if err := json.Unmarshal(data, &payload); err != nil {
		return fmt.Errorf("failed to unmarshal PayloadSkeleton: %w", err)
	}

	if sessionInfo.ClientUUID == "" || payload.UserID != "" {
		sessionInfo.ClientUUID = payload.UserID
	}

	// Handshake check
	if payload.PayloadType == "Handshake" && sessionInfo.HandshakeDone {
		log.Println("[WARN] Handshake already done, ignoring Handshake payload")
		return nil
	} else if payload.PayloadType != "Handshake" && !sessionInfo.HandshakeDone {
		res := fmt.Sprintf("handshake not done, ignoring %s payload", payload.RequestID)
		log.Println(res)
		errorPayload := ApiCommonTypes.ErrorPayload{
			Error: res,
		}

		resPayload, err := PayloadUtils.EncryptInternalPayload(sessionInfo, payload.RequestID, "error", errorPayload)
		if err != nil {
			return err
		}

		err = sessionInfo.Conn.WriteMessage(websocket.TextMessage, resPayload)
		if err != nil {
			return err
		}
		return nil
	}

	// Rufe den entsprechenden Handler auf
	handler, ok := payloadHandlers[payload.PayloadType]
	if !ok {
		log.Printf("[WARN] Unknown package type: %s", payload.PayloadType)
		return nil
	}

	if err := handler(sessionInfo, payload); err != nil {
		log.Printf("[ERROR] Handler for %s failed: %v", payload.PayloadType, err)
		return err
	}
	return nil
}
