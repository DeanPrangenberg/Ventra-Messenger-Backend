package gRPCserver_test

import (
	"context"
	"crypto/ecdh"
	"encoding/base64"
	"encoding/json"
	"testing"

	"CryptoLib/src"
	VM_AUTH_gRPC_Wrapper "VM-AUTH-gRPC-Wrapper/src"
	"VM-AUTH/src/AuthCommonTypes"
)

func encryptPayload(sharedSecret []byte, payload []byte) (encrypted []byte, iv []byte, err error) {
	iv, err = CryptoLib.GenRandomBytes(12)
	if err != nil {
		return nil, nil, err
	}
	encrypted, err = CryptoLib.EncryptGCM(sharedSecret, iv, payload)
	return
}

func decryptResponse(sharedSecret []byte, iv []byte, response []byte) ([]byte, error) {
	return CryptoLib.DecryptGCM(sharedSecret, iv, response)
}

func TestFullAuthFlowWithEncryption(t *testing.T) {
	client, err := VM_AUTH_gRPC_Wrapper.NewVmAuthClient("localhost:4445")
	if err != nil {
		t.Fatalf("Failed to create client: %v", err)
	}
	defer func() {
		if err := client.Close(); err != nil {
			t.Logf("client close error: %v", err)
		}
	}()

	userId := "test-user"
	email := "test@example.com"
	password := "password123"
	userName := "tester"

	ctx := context.Background()

	// 1. Generate X25519 key pair
	priv, pub, err := CryptoLib.GenerateX25519KeyPair()
	if err != nil {
		t.Fatalf("Key generation failed: %v", err)
	}
	clientPubKeyBytes := pub.Bytes()
	clientPubKeyBase64 := base64.StdEncoding.EncodeToString(clientPubKeyBytes)

	// 2. Handshake
	authPubKeyBase64, err := client.Handshake(ctx, "", userId, clientPubKeyBase64)
	if err != nil {
		t.Fatalf("Handshake failed: %v", err)
	}
	authPubKeyBytes, err := base64.StdEncoding.DecodeString(authPubKeyBase64)
	if err != nil {
		t.Fatalf("Failed to decode server pubkey: %v", err)
	}
	serverPub, err := ecdh.X25519().NewPublicKey(authPubKeyBytes)
	if err != nil {
		t.Fatalf("Failed to parse server pubkey: %v", err)
	}
	sharedSecret, err := CryptoLib.ComputeSharedSecret(priv, serverPub)
	if err != nil {
		t.Fatalf("Failed to compute shared secret: %v", err)
	}

	// Server stores the Blake2s hash of the raw ECDH shared secret as the AES key.
	// Mirror that here so encryption/decryption keys match.
	sharedSecret = CryptoLib.Blake2sSum256(sharedSecret)

	// Helper to send encrypted payload and decrypt response
	sendEncryptedPayload := func(payloadType string, payload interface{}, out interface{}) {
		internalPayload, _ := json.Marshal(payload)
		payloadSkeleton := AuthCommonTypes.PayloadSkeleton{
			PayloadType:     payloadType,
			InternalPayload: internalPayload,
		}

		payloadBytes, _ := json.Marshal(payloadSkeleton)
		encrypted, iv, err := encryptPayload(sharedSecret, payloadBytes)
		if err != nil {
			t.Fatalf("Encryption failed: %v", err)
		}
		// Prefix IV to the ciphertext so server can extract it
		payloadToSend := append(iv, encrypted...)
		respBytes, err := client.SendPayload(ctx, "", userId, payloadToSend)
		if err != nil {
			t.Fatalf("%s failed: %v", payloadType, err)
		}
		decrypted, err := decryptResponse(sharedSecret, iv, respBytes)
		if err != nil {
			t.Fatalf("Decryption failed: %v", err)
		}
		if err := json.Unmarshal(decrypted, out); err != nil {
			t.Fatalf("Unmarshal failed: %v", err)
		}
	}

	// 3. Register
	var registerResp AuthCommonTypes.RegisterPayloadResponse
	sendEncryptedPayload("Register", AuthCommonTypes.RegisterPayloadRequest{
		UserName: userName,
		EMail:    email,
		Password: password,
	}, &registerResp)
	if !registerResp.Success {
		t.Fatalf("Register response not successful")
	}

	// 4. Login
	var loginResp AuthCommonTypes.LoginPayloadResponse
	sendEncryptedPayload("Login", AuthCommonTypes.LoginPayloadRequest{
		EMail:    email,
		Password: password,
	}, &loginResp)
	if !loginResp.Success {
		t.Fatalf("Login response not successful")
	}
	if loginResp.UserID != registerResp.UserID {
		t.Fatalf("UserID mismatch between register and login")
	}

	// 5. NewSessionToken
	var newTokenResp AuthCommonTypes.NewSessionTokenPayloadResponse
	sendEncryptedPayload("NewSessionToken", AuthCommonTypes.NewSessionTokenPayloadRequest{
		LongTimeToken: loginResp.LongTimeToken,
	}, &newTokenResp)
	if newTokenResp.SessionToken == "" {
		t.Fatalf("NewSessionToken response empty")
	}

	// 6. UpdateSessionToken
	var updateTokenResp AuthCommonTypes.UpdateSessionTokenPayloadResponse
	sendEncryptedPayload("UpdateSessionToken", AuthCommonTypes.UpdateSessionTokenPayloadRequest{
		OldSessionToken: newTokenResp.SessionToken,
	}, &updateTokenResp)
	if updateTokenResp.NewSessionToken == "" {
		t.Fatalf("UpdateSessionToken response empty")
	}

	// 7. VerifyToken
	valid, err := client.VerifyToken(ctx, updateTokenResp.NewSessionToken, userId)
	if err != nil {
		t.Fatalf("VerifyToken failed: %v", err)
	}
	if !valid {
		t.Fatalf("Token verification failed")
	}
}
