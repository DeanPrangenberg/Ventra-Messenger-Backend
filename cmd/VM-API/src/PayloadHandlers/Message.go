package PayloadHandlers

import (
	CryptoLib "CryptoLib/src"
	"VM-API/src/ApiCommonTypes"
	"encoding/base64"
	"encoding/json"
	"log"
)

func handleEncryptedMessage(sessionInfo *ApiCommonTypes.WebSocketSession, payloadSkeleton ApiCommonTypes.PayloadSkeleton) (error, ApiCommonTypes.MessagePayload) {
	iv, err := base64.StdEncoding.DecodeString(payloadSkeleton.IV)
	if err != nil {
		log.Printf("[ERROR] Failed to decode IV: %v", err)
		return err, ApiCommonTypes.MessagePayload{}
	}

	var ciphertextBase64 string
	if err := json.Unmarshal(payloadSkeleton.InternalPayload, &ciphertextBase64); err != nil {
		log.Printf("[ERROR] Failed to parse ciphertext: %v", err)
		return err, ApiCommonTypes.MessagePayload{}
	}

	ciphertext, err := base64.StdEncoding.DecodeString(ciphertextBase64)
	if err != nil {
		log.Printf("[ERROR] Failed to decode ciphertext: %v", err)
		return err, ApiCommonTypes.MessagePayload{}
	}

	decryptedBytes, err := CryptoLib.DecryptGCM(sessionInfo.SharedSecret, iv, ciphertext)
	if err != nil {
		log.Printf("[ERROR] Decryption failed: %v", err)
		return err, ApiCommonTypes.MessagePayload{}
	}

	var msg ApiCommonTypes.MessagePayload
	if err := json.Unmarshal(decryptedBytes, &msg); err != nil {
		log.Printf("[ERROR] Failed to unmarshal decrypted message: %v", err)
		return err, ApiCommonTypes.MessagePayload{}
	}

	return nil, msg
}
