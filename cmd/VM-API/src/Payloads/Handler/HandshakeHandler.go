package Handler

import (
	CryptoLib "CryptoLib/src"
	"VM-API/src/ApiCommonTypes"
	"crypto/ecdh"
	"encoding/base64"
	"encoding/json"
	"log"

	"github.com/gorilla/websocket"
)

func HandshakeHandler(session *ApiCommonTypes.WebSocketSession, payload ApiCommonTypes.PayloadSkeleton) error {
	var clientPubKeyBase64 string
	if err := json.Unmarshal(payload.InternalPayload, &clientPubKeyBase64); err != nil {
		log.Printf("[ERROR] Failed to parse client public key: %v", err)
		return err
	}

	clientPubKeyBytes, err := base64.StdEncoding.DecodeString(clientPubKeyBase64)
	if err != nil {
		log.Printf("[ERROR] Failed to decode client public key: %v", err)
		return err
	}

	clientPubKey, err := ecdh.X25519().NewPublicKey(clientPubKeyBytes)
	if err != nil {
		log.Printf("[ERROR] Invalid client public key: %v", err)
		return err
	}

	if session.PrivKey == nil || session.PubKey == nil {
		privKey, pubKey, err := CryptoLib.GenerateX25519KeyPair()
		if err != nil {
			log.Printf("[ERROR] Failed to generate server key pair: %v", err)
			return err
		}
		session.PrivKey = privKey
		session.PubKey = pubKey
	}

	// After computing the shared secret:
	sharedSecret, err := CryptoLib.ComputeSharedSecret(session.PrivKey, clientPubKey)
	if err != nil {
		log.Printf("[ERROR] Failed to compute shared secret: %v", err)
		return err
	}

	// Hash the shared secret and store the hash
	hashedSecret := CryptoLib.Blake2sSum256(sharedSecret)

	session.SharedSecret = hashedSecret
	session.HandshakeDone = true

	serverPubKeyBase64 := base64.StdEncoding.EncodeToString(session.PubKey.Bytes())

	response := map[string]string{
		"type":         "HandshakeAck",
		"msg":          "Handshake successful",
		"serverPubKey": serverPubKeyBase64,
	}
	respBytes, err := json.Marshal(response)
	if err != nil {
		log.Printf("[ERROR] Failed to marshal handshake response: %v", err)
	}

	if err := session.Conn.WriteMessage(websocket.TextMessage, respBytes); err != nil {
		log.Printf("[ERROR] Failed to send handshake response: %v", err)
		return err
	}

	log.Println("[INFO] Handshake done with client:", session.ClientUUID)
	return nil
}
