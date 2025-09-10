package PayloadHandlers

import (
	"VM-API/src/ApiCommonTypes"
	"encoding/json"
	"log"
)

func ProcessPkg(sessionInfo *ApiCommonTypes.WebSocketSession, data []byte) error {
	var pkg ApiCommonTypes.PayloadSkeleton
	if err := json.Unmarshal(data, &pkg); err != nil {
		log.Printf("[ERROR] Failed to unmarshal package: %v", err)
		return err
	}

	switch pkg.PayloadType {
	case "Handshake":
		return handleHandshake(sessionInfo, pkg.InternalPayload)
	case "Message":
		if !sessionInfo.APIHandshakeDone {
			log.Println("[WARN] Handshake not done, ignoring Message")
			return nil
		}
		err, msg := handleEncryptedMessage(sessionInfo, pkg)
		if err != nil {
			log.Printf("[ERROR] Failed to handle encrypted message: %v", err)
			return err
		}

		// TODO: Implement message sending logic
		log.Printf("[INFO] Message sent: %s", msg.Content)
		return nil
	case "Login":
		if !sessionInfo.APIHandshakeDone {
			log.Println("[WARN] Handshake not done, ignoring AuthHandshake")
			return nil
		}
		return nil

	case "AuthPayload":
		if !sessionInfo.APIHandshakeDone {
			log.Println("[WARN] Handshake not done, ignoring AuthRequest")
			return nil
		}
		return nil

	default:
		log.Printf("[WARN] Unknown package type: %s", pkg.PayloadType)
		return nil
	}
}
