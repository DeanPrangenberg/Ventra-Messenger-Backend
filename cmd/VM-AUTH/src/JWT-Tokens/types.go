package JWT_Tokens

import (
	"github.com/golang-jwt/jwt/v5"
)

// Claims definiert die Struktur der JWT-Claims
type Claims struct {
	UserID    string `json:"user_id"`
	TokenType string `json:"token_type"`
	jwt.RegisteredClaims
}
