package PayloadUtils

import (
	CryptoLib "CryptoLib/src"
	"VM-API/src/ApiCommonTypes"
	"encoding/base64"
	"encoding/json"
	"fmt"
	"log"
)

func decryptInternalPayload(session *ApiCommonTypes.WebSocketSession, payload *ApiCommonTypes.PayloadSkeleton) error {
	if !requireHandshake(session) {
		return nil
	}

	iv, err := base64.StdEncoding.DecodeString(payload.IV)
	if err != nil {
		log.Printf("[ERROR] Failed to decode IV: %v", err)
		return err
	}

	var ciphertextBase64 string
	if err := json.Unmarshal(payload.InternalPayload, &ciphertextBase64); err != nil {
		log.Printf("[ERROR] Failed to parse ciphertext: %v", err)
		return err
	}

	ciphertext, err := base64.StdEncoding.DecodeString(ciphertextBase64)
	if err != nil {
		log.Printf("[ERROR] Failed to decode ciphertext: %v", err)
		return err
	}

	decryptedBytes, err := CryptoLib.DecryptGCM(session.SharedSecret, iv, ciphertext)
	if err != nil {
		log.Printf("[ERROR] Decryption failed: %v", err)
		return err
	}

	payload.InternalPayload = decryptedBytes
	return nil
}

func EncryptInternalPayload(session *ApiCommonTypes.WebSocketSession, RequestID, payloadType string, internalPayload interface{}) ([]byte, error) {
	if !requireHandshake(session) {
		return nil, nil
	}

	internalPayloadBytes, err := json.Marshal(internalPayload)
	if err != nil {
		log.Printf("[ERROR] Failed to marshal internal payload: %v", err)
		return nil, err
	}

	iv, err := CryptoLib.GenRandomBytes(12)
	if err != nil {
		log.Printf("[ERROR] Failed to generate IV: %v", err)
		return nil, err
	}

	ciphertext, err := CryptoLib.EncryptGCM(session.SharedSecret, iv, internalPayloadBytes)
	if err != nil {
		log.Printf("[ERROR] Encryption failed: %v", err)
		return nil, err
	}

	payload := ApiCommonTypes.PayloadSkeleton{
		RequestID:       RequestID,
		PayloadType:     payloadType,
		IV:              base64.StdEncoding.EncodeToString(iv),
		InternalPayload: []byte(fmt.Sprintf("\"%s\"", base64.StdEncoding.EncodeToString(ciphertext))),
	}

	payloadBytes, err := json.Marshal(payload)
	if err != nil {
		return nil, err
	}

	return payloadBytes, nil
}
