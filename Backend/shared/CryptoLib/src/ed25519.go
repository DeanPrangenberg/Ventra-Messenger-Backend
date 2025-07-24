package src

import (
	"crypto/ed25519"
	"crypto/rand"
)

type Ed25519KeyPair struct {
	PublicKey  ed25519.PublicKey
	PrivateKey ed25519.PrivateKey
}

func (EdKeyPair *Ed25519KeyPair) GenerateEd25519KeyPair() error {
	var err error
	EdKeyPair.PublicKey, EdKeyPair.PrivateKey, err = ed25519.GenerateKey(rand.Reader)
	return err
}

func (EdKeyPair *Ed25519KeyPair) SignEd25519(message []byte) []byte {
	return ed25519.Sign(EdKeyPair.PrivateKey, message)
}

func (EdKeyPair *Ed25519KeyPair) VerifyEd25519(message, signature []byte) bool {
	return ed25519.Verify(EdKeyPair.PublicKey, message, signature)
}
