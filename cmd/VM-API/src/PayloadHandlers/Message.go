package PayloadHandlers

import (
	CryptoLib "CryptoLib/src"
	"VM-API/src/commonTypes"
	"encoding/base64"
	"encoding/json"
	"log"
)

func handleEncryptedMessage(sessionInfo *commonTypes.WebSocketSession, pkg commonTypes.Pkg) (error, commonTypes.MessagePkg) {

	iv, err := base64.StdEncoding.DecodeString(pkg.IV)
	if err != nil {
		log.Printf("[ERROR] Failed to decode IV: %v", err)
		return err, commonTypes.MessagePkg{}
	}

	var ciphertextBase64 string
	if err := json.Unmarshal(pkg.Pkg, &ciphertextBase64); err != nil {
		log.Printf("[ERROR] Failed to parse ciphertext: %v", err)
		return err, commonTypes.MessagePkg{}
	}

	ciphertext, err := base64.StdEncoding.DecodeString(ciphertextBase64)
	if err != nil {
		log.Printf("[ERROR] Failed to decode ciphertext: %v", err)
		return err, commonTypes.MessagePkg{}
	}

	decryptedBytes, err := CryptoLib.DecryptGCM(sessionInfo.SharedSecret, iv, ciphertext)
	if err != nil {
		log.Printf("[ERROR] Decryption failed: %v", err)
		return err, commonTypes.MessagePkg{}
	}

	var msg commonTypes.MessagePkg
	if err := json.Unmarshal(decryptedBytes, &msg); err != nil {
		log.Printf("[ERROR] Failed to unmarshal decrypted message: %v", err)
		return err, commonTypes.MessagePkg{}
	}

	return nil, msg
}
