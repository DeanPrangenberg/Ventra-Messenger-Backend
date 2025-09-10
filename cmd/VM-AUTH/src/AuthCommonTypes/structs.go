package AuthCommonTypes

import (
	"crypto/ecdh"
)

type UserSession struct {
	UserID            string `json:"userId"`
	SharedSecret      []byte
	PrivKey           *ecdh.PrivateKey
	PubKey            *ecdh.PublicKey
	AUTHHandshakeDone bool
}
