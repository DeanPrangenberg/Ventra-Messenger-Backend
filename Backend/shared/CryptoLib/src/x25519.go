package CryptoLib

import (
	"crypto/ecdh"
	"crypto/rand"
)

// GenerateX25519KeyPair generates a new X25519 key pair.
func GenerateX25519KeyPair() (priv *ecdh.PrivateKey, pub *ecdh.PublicKey, err error) {
	priv, err = ecdh.X25519().GenerateKey(rand.Reader)
	if err != nil {
		return nil, nil, err
	}
	return priv, priv.PublicKey(), nil
}

// ComputeSharedSecret computes the shared secret using your private key and the peer's public key.
func ComputeSharedSecret(priv *ecdh.PrivateKey, peerPub *ecdh.PublicKey) ([]byte, error) {
	return priv.ECDH(peerPub)
}
