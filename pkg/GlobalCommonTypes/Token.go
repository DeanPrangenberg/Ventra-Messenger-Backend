package GlobalCommonTypes

import "time"

type TokenRecord struct {
	ID          string    `json:"id"`
	UserID      string    `json:"user_id"`
	TokenType   string    `json:"token_type"`
	CreatedAt   time.Time `json:"created_at"`
	ExpiresAt   time.Time `json:"expires_at"`
	ParentToken string    `json:"parent_token,omitempty"`
	Revoked     bool      `json:"revoked"`
	LastUsedAt  time.Time `json:"last_used_at"`
}
