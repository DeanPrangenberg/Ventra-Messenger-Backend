package CryptoLib

import (
	"crypto/aes"
	"crypto/cipher"
	"errors"
)

// EncryptGCM encrypts plaintext using AES-256-GCM with a provided nonce.
func EncryptGCM(key, iv, plaintext []byte) ([]byte, error) {
	block, err := aes.NewCipher(key)
	if err != nil {
		return nil, err
	}
	gcm, err := cipher.NewGCM(block)
	if err != nil {
		return nil, err
	}
	if len(iv) != gcm.NonceSize() {
		return nil, errors.New("invalid nonce size")
	}
	ciphertext := gcm.Seal(nil, iv, plaintext, nil)
	return ciphertext, nil
}

// DecryptGCM decrypts ciphertext using AES-256-GCM with a provided nonce.
func DecryptGCM(key, iv, ciphertext []byte) ([]byte, error) {
	block, err := aes.NewCipher(key)
	if err != nil {
		return nil, err
	}
	gcm, err := cipher.NewGCM(block)
	if err != nil {
		return nil, err
	}
	if len(iv) != gcm.NonceSize() {
		return nil, errors.New("invalid nonce size")
	}
	plaintext, err := gcm.Open(nil, iv, ciphertext, nil)
	if err != nil {
		return nil, err
	}
	return plaintext, nil
}
