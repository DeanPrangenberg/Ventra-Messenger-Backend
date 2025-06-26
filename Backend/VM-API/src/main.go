package main

import (
	"VM-API/src/NetworkPackets"
	"VM-API/src/crypto"
	"crypto/ecdh"
	"encoding/base64"
	"encoding/json"
	"github.com/gorilla/websocket"
	"log"
	"net/http"
)

var Debug = true

func debugLog(format string, v ...interface{}) {
	if Debug {
		log.Printf("[DEBUG] "+format, v...)
	}
}

type Session struct {
	Conn          *websocket.Conn
	SharedSecret  []byte
	handShakeDone bool
	privKey       *ecdh.PrivateKey
	pubKey        *ecdh.PublicKey
}

var upgrader = websocket.Upgrader{
	CheckOrigin: func(r *http.Request) bool { return true },
}

func processPkg(session *Session, data []byte) error {
	var pkg NetworkPackets.Pkg
	if err := json.Unmarshal(data, &pkg); err != nil {
		log.Printf("[ERROR] Failed to unmarshal package: %v", err)
		return err
	}

	switch pkg.MsgType {
	case "Handshake":
		return handleHandshake(session, pkg.Pkg)

	case "MessagePkg":
		if !session.handShakeDone {
			log.Println("[WARN] Handshake not done, ignoring MessagePkg")
			return nil
		}
		return handleEncryptedMessage(session, pkg)

	default:
		log.Printf("[WARN] Unknown package type: %s", pkg.MsgType)
		return nil
	}
}

func handleHandshake(session *Session, pkgData json.RawMessage) error {
	var clientPubKeyBase64 string
	if err := json.Unmarshal(pkgData, &clientPubKeyBase64); err != nil {
		log.Printf("[ERROR] Failed to parse client public key: %v", err)
		return err
	}

	debugLog("Client public key (base64): %s", clientPubKeyBase64)

	clientPubKeyBytes, err := base64.StdEncoding.DecodeString(clientPubKeyBase64)
	if err != nil {
		log.Printf("[ERROR] Failed to decode client public key: %v", err)
		return err
	}

	debugLog("Client public key (bytes): %x", clientPubKeyBytes)

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
		debugLog("Server private key: %x", session.privKey.Bytes())
		debugLog("Server public key: %x", session.pubKey.Bytes())
	}

	// After computing the shared secret:
	sharedSecret, err := crypto.ComputeSharedSecret(session.privKey, clientPubKey)
	if err != nil {
		log.Printf("[ERROR] Failed to compute shared secret: %v", err)
		return err
	}
	debugLog("Shared secret (raw): %x", sharedSecret)

	// Hash the shared secret and store the hash
	hashedSecret := crypto.Blake2sSum256(sharedSecret)
	debugLog("Shared secret (hashed): %x", hashedSecret)

	session.SharedSecret = hashedSecret
	session.handShakeDone = true

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

func handleEncryptedMessage(session *Session, pkg NetworkPackets.Pkg) error {
	debugLog("Received IV (base64): %s", pkg.IV)
	debugLog("Session shared secret: %x", session.SharedSecret)

	iv, err := base64.StdEncoding.DecodeString(pkg.IV)
	if err != nil {
		log.Printf("[ERROR] Failed to decode IV: %v", err)
		return err
	}
	debugLog("IV (bytes): %x", iv)

	var ciphertextBase64 string
	if err := json.Unmarshal(pkg.Pkg, &ciphertextBase64); err != nil {
		log.Printf("[ERROR] Failed to parse ciphertext: %v", err)
		return err
	}
	debugLog("Ciphertext (base64): %s", ciphertextBase64)

	ciphertext, err := base64.StdEncoding.DecodeString(ciphertextBase64)
	if err != nil {
		log.Printf("[ERROR] Failed to decode ciphertext: %v", err)
		return err
	}
	debugLog("Ciphertext (bytes): %x", ciphertext)

	debugLog("Decryption key: %x", session.SharedSecret)

	decryptedBytes, err := crypto.DecryptGCM(session.SharedSecret, iv, ciphertext)
	if err != nil {
		log.Printf("[ERROR] Decryption failed: %v", err)
		return err
	}

	log.Println("[INFO] Message decrypted successfully")
	log.Println("[INFO] Decrypted message content:", string(decryptedBytes))

	return nil
}

func wsHandler(w http.ResponseWriter, r *http.Request) {
	conn, err := upgrader.Upgrade(w, r, nil)
	if err != nil {
		log.Printf("[ERROR] WebSocket upgrade failed: %v", err)
		return
	}
	defer conn.Close()
	log.Printf("[INFO] Client connected: %s", r.RemoteAddr)

	session := &Session{Conn: conn}

	for {
		mt, msg, err := conn.ReadMessage()
		if err != nil {
			log.Printf("[ERROR] Read error: %v", err)
			break
		}

		if err := processPkg(session, msg); err != nil {
			log.Printf("[ERROR] Processing package failed: %v", err)
			break
		}

		if err := conn.WriteMessage(mt, msg); err != nil {
			log.Printf("[ERROR] Write error: %v", err)
			break
		}
	}
}

func main() {
	http.HandleFunc("/ws", wsHandler)
	log.Println("[INFO] Server running on :8881")
	log.Fatal(http.ListenAndServe(":8881", nil))
}
