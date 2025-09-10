package JWT_Tokens

import (
	"errors"
	"fmt"
	"time"

	"github.com/golang-jwt/jwt/v5"
)

func (tm *TokenManager) VerifyToken(tokenString string) (*Claims, error) {
	claims, err := tm.verifyToken(tokenString)
	if err != nil {
		return nil, err
	}
	tokenRecord, err := tm.dbManager.LoadToken(claims.ID)
	if err == nil {
		if tokenRecord.Revoked {
			return nil, errors.New("Token ist gesperrt")
		}
		tokenRecord.LastUsedAt = time.Now()
	}
	return claims, nil
}

func (tm *TokenManager) verifyToken(tokenString string) (*Claims, error) {
	claims := &Claims{}
	token, err := jwt.ParseWithClaims(tokenString, claims, func(token *jwt.Token) (interface{}, error) {
		if _, ok := token.Method.(*jwt.SigningMethodRSA); !ok {
			return nil, fmt.Errorf("unerwartete Signing-Methode: %v", token.Header["alg"])
		}
		return tm.publicKey, nil
	})
	if err != nil {
		return nil, err
	}
	if !token.Valid {
		return nil, errors.New("ungültiger Token")
	}
	if claims.ExpiresAt.Before(time.Now()) {
		return nil, errors.New("Token ist abgelaufen")
	}
	return claims, nil
}
