package NetworkPackets

import (
	"VM-API/src/crypto"
	"VM-API/src/logs"
	"crypto/ecdh"
	"encoding/base64"
	"encoding/json"
	"github.com/gorilla/websocket"
	"log"
)

func handleHandshake(session *WebSocketSession, pkgData json.RawMessage) error {
	var clientPubKeyBase64 string
	if err := json.Unmarshal(pkgData, &clientPubKeyBase64); err != nil {
		log.Printf("[ERROR] Failed to parse client public key: %v", err)
		return err
	}

	logs.DebugLog("Client public key (base64): %s", clientPubKeyBase64)

	clientPubKeyBytes, err := base64.StdEncoding.DecodeString(clientPubKeyBase64)
	if err != nil {
		log.Printf("[ERROR] Failed to decode client public key: %v", err)
		return err
	}

	logs.DebugLog("Client public key (hex): %x", clientPubKeyBytes)

	clientPubKey, err := ecdh.X25519().NewPublicKey(clientPubKeyBytes)
	if err != nil {
		log.Printf("[ERROR] Invalid client public key: %v", err)
		return err
	}

	if session.privKey == nil || session.pubKey == nil {
		privKey, pubKey, err := crypto.GenerateX25519KeyPair()
		if err != nil {
			log.Printf("[ERROR] Failed to generate server key pair: %v", err)
			return err
		}
		session.privKey = privKey
		session.pubKey = pubKey
		logs.DebugLog("Server private key: %x", session.privKey.Bytes())
		logs.DebugLog("Server public key: %x", session.pubKey.Bytes())
	}

	// After computing the shared secret:
	sharedSecret, err := crypto.ComputeSharedSecret(session.privKey, clientPubKey)
	if err != nil {
		log.Printf("[ERROR] Failed to compute shared secret: %v", err)
		return err
	}
	logs.DebugLog("Shared secret (raw): %x", sharedSecret)

	// Hash the shared secret and store the hash
	hashedSecret := crypto.Blake2sSum256(sharedSecret)
	logs.DebugLog("Shared secret (hashed): %x", hashedSecret)

	session.SharedSecret = hashedSecret
	session.HandShakeDone = true

	serverPubKeyBase64 := base64.StdEncoding.EncodeToString(session.pubKey.Bytes())

	response := map[string]string{
		"type":         "HandshakeAck",
		"msg":          "Handshake successful",
		"serverPubKey": serverPubKeyBase64,
	}
	respBytes, _ := json.Marshal(response)

	if err := session.Conn.WriteMessage(websocket.TextMessage, respBytes); err != nil {
		log.Printf("[ERROR] Failed to send handshake response: %v", err)
		return err
	}

	log.Println("[INFO] Handshake successful")
	return nil
}

func handleEncryptedMessage(sessionInfo *WebSocketSession, pkg Pkg) error {
	logs.DebugLog("Received IV (base64): %s", pkg.IV)
	logs.DebugLog("Session shared secret: %x", sessionInfo.SharedSecret)

	iv, err := base64.StdEncoding.DecodeString(pkg.IV)
	if err != nil {
		log.Printf("[ERROR] Failed to decode IV: %v", err)
		return err
	}
	logs.DebugLog("IV (hex): %x", iv)

	var ciphertextBase64 string
	if err := json.Unmarshal(pkg.Pkg, &ciphertextBase64); err != nil {
		log.Printf("[ERROR] Failed to parse ciphertext: %v", err)
		return err
	}
	logs.DebugLog("Ciphertext (base64): %s", ciphertextBase64)

	ciphertext, err := base64.StdEncoding.DecodeString(ciphertextBase64)
	if err != nil {
		log.Printf("[ERROR] Failed to decode ciphertext: %v", err)
		return err
	}
	logs.DebugLog("Ciphertext (hex): %x", ciphertext)

	logs.DebugLog("Decryption key: %x", sessionInfo.SharedSecret)

	decryptedBytes, err := crypto.DecryptGCM(sessionInfo.SharedSecret, iv, ciphertext)
	if err != nil {
		log.Printf("[ERROR] Decryption failed: %v", err)
		return err
	}

	log.Println("[INFO] Message decrypted successfully")
	log.Println("[INFO] Decrypted message content:", string(decryptedBytes))

	return nil
}
