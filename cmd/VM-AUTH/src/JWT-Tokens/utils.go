package JWT_Tokens

import (
	"crypto/rand"
	"fmt"
)

// generateTokenID generiert eine eindeutige Token-ID
func generateTokenID() string {
	b := make([]byte, 64)
	rand.Read(b)
	return fmt.Sprintf("%x", b)
}
