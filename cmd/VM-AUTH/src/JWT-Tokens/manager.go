package JWT_Tokens

import (
	"GlobalCommonTypes"
	PostgresWrapper "PostgresWrapper/src"
	"crypto/rand"
	"crypto/rsa"
	"errors"
	"fmt"
	"time"

	"github.com/golang-jwt/jwt/v5"
)

// TokenManager verwaltet die JWT-Token mit Ausfallsicherheit
type TokenManager struct {
	privateKey *rsa.PrivateKey
	publicKey  *rsa.PublicKey
	config     *JWTConfig
	dbManager  *PostgresWrapper.DB
}

// NewTokenManager erstellt einen neuen TokenManager
func NewTokenManager(config *JWTConfig, db *PostgresWrapper.DB) (*TokenManager, error) {
	privateKey, err := rsa.GenerateKey(rand.Reader, 4096)
	if err != nil {
		return nil, err
	}
	return &TokenManager{
		privateKey: privateKey,
		publicKey:  &privateKey.PublicKey,
		config:     config,
		dbManager:  db,
	}, nil
}

func (tm *TokenManager) CreateRefreshToken(userID string) (string, error) {
	tokenID := generateTokenID()
	duration := tm.config.RefreshDuration()
	claims := Claims{
		UserID:    userID,
		TokenType: "longtime",
		RegisteredClaims: jwt.RegisteredClaims{
			ID:        tokenID,
			ExpiresAt: jwt.NewNumericDate(time.Now().Add(duration)),
			IssuedAt:  jwt.NewNumericDate(time.Now()),
			Subject:   userID,
		},
	}
	token := jwt.NewWithClaims(jwt.SigningMethodRS256, claims)
	tokenString, err := token.SignedString(tm.privateKey)
	if err != nil {
		return "", err
	}
	record := &GlobalCommonTypes.TokenRecord{
		ID:         tokenID,
		UserID:     userID,
		TokenType:  "longtime",
		CreatedAt:  time.Now(),
		ExpiresAt:  time.Now().Add(duration),
		Revoked:    false,
		LastUsedAt: time.Now(),
	}
	err = tm.dbManager.AddToken(record)
	if err != nil {
		fmt.Printf("Warning: Saving Token in DB faild: %v\n", err)
	}
	return tokenString, nil
}

func (tm *TokenManager) CreateSessionToken(longTimeToken string) (string, error) {
	claims, err := tm.verifyToken(longTimeToken)
	if err != nil {
		return "", fmt.Errorf("invalid RefreshToken: %w", err)
	}
	if claims.TokenType != "longtime" {
		return "", errors.New("token is not a RefreshToken")
	}
	if claims.ExpiresAt.Before(time.Now()) {
		return "", errors.New("refreshToken is expired")
	}
	longTokenRecord, err := tm.dbManager.LoadToken(claims.ID)
	if err == nil && longTokenRecord.Revoked {
		return "", errors.New("refreshToken is revoked")
	}
	duration := tm.config.SessionDuration()
	sessionTokenID := generateTokenID()
	sessionClaims := Claims{
		UserID:    claims.UserID,
		TokenType: "session",
		RegisteredClaims: jwt.RegisteredClaims{
			ID:        sessionTokenID,
			ExpiresAt: jwt.NewNumericDate(time.Now().Add(duration)),
			IssuedAt:  jwt.NewNumericDate(time.Now()),
			Subject:   claims.UserID,
		},
	}
	token := jwt.NewWithClaims(jwt.SigningMethodRS256, sessionClaims)
	tokenString, err := token.SignedString(tm.privateKey)
	if err != nil {
		return "", err
	}
	sessionRecord := &GlobalCommonTypes.TokenRecord{
		ID:          sessionTokenID,
		UserID:      claims.UserID,
		TokenType:   "session",
		CreatedAt:   time.Now(),
		ExpiresAt:   time.Now().Add(duration),
		ParentToken: claims.ID,
		Revoked:     false,
		LastUsedAt:  time.Now(),
	}
	err = tm.dbManager.AddToken(sessionRecord)
	if err != nil {
		fmt.Printf("Warning: Saving SessionToken in DB faild: %v\n", err)
	}
	return tokenString, nil
}
