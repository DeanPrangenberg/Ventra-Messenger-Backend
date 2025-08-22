package PayloadHandlers

import (
	"VM-API/src/commonTypes"
	"encoding/json"
	"log"
)

func ProcessPkg(sessionInfo *commonTypes.WebSocketSession, data []byte) error {
	var pkg commonTypes.Pkg
	if err := json.Unmarshal(data, &pkg); err != nil {
		log.Printf("[ERROR] Failed to unmarshal package: %v", err)
		return err
	}

	switch pkg.MsgType {
	case "Handshake":
		return handleHandshake(sessionInfo, pkg.Pkg)
	case "MessagePkg":
		if !sessionInfo.HandShakeDone {
			log.Println("[WARN] Handshake not done, ignoring MessagePkg")
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

	default:
		log.Printf("[WARN] Unknown package type: %s", pkg.MsgType)
		return nil
	}
}
