package GlobalCommonTypes

import "time"

// AccountRecord represents a user stored in the database.
type AccountRecord struct {
	UserID       string    `json:"user_id"`
	Username     string    `json:"user_name"`
	Email        string    `json:"email"`
	PasswordHash string    `json:"password_hash"`
	CreatedAt    time.Time `json:"created_at"`
	Revoked      bool      `json:"revoked"`
	LastLogin    time.Time `json:"last_login,omitempty"`
}
