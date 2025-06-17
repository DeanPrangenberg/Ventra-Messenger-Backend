package crypto

import (
	"golang.org/x/crypto/blake2s"
)

func Blake2sSum256(input []byte) []byte {
	sum := blake2s.Sum256(input)
	return sum[:]
}
