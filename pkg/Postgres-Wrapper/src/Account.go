package PostgresWrapper

import (
	"GlobalCommonTypes"
	"database/sql"
	"fmt"
)

func (c *DB) AddAccount(data *GlobalCommonTypes.AccountRecord) error {
	query := `
	INSERT INTO users (user_id, user_name, email, password_hash, created_at, revoked, last_login)
	VALUES ($1, $2, $3, $4, $5, $6, $7);
	`

	var lastLogin interface{}
	if !data.LastLogin.IsZero() {
		lastLogin = data.LastLogin
	} else {
		lastLogin = nil
	}

	_, err := c.Exec(query,
		data.UserID,
		data.Username,
		data.Email,
		data.PasswordHash,
		data.CreatedAt,
		data.Revoked,
		lastLogin,
	)

	if err != nil {
		return fmt.Errorf("failed to add user: %w", err)
	}
	return nil
}

func (c *DB) LoadAccountByID(userID string) (*GlobalCommonTypes.AccountRecord, error) {
	query := `SELECT user_id, user_name, email, password_hash, created_at, revoked, last_login FROM users WHERE user_id = $1;`
	row := c.QueryRow(query, userID)

	var u GlobalCommonTypes.AccountRecord
	var lastLogin sql.NullTime
	err := row.Scan(
		&u.UserID,
		&u.Username,
		&u.Email,
		&u.PasswordHash,
		&u.CreatedAt,
		&u.Revoked,
		&lastLogin,
	)
	if err != nil {
		return nil, fmt.Errorf("failed to load user by id: %w", err)
	}
	if lastLogin.Valid {
		u.LastLogin = lastLogin.Time
	}
	return &u, nil
}

func (c *DB) LoadAccountByEmail(email string) (*GlobalCommonTypes.AccountRecord, error) {
	query := `SELECT user_id, user_name, email, password_hash, created_at, revoked, last_login FROM users WHERE email = $1;`
	row := c.QueryRow(query, email)

	var u GlobalCommonTypes.AccountRecord
	var lastLogin sql.NullTime
	err := row.Scan(
		&u.UserID,
		&u.Username,
		&u.Email,
		&u.PasswordHash,
		&u.CreatedAt,
		&u.Revoked,
		&lastLogin,
	)
	if err != nil {
		return nil, fmt.Errorf("failed to load user by email: %w", err)
	}
	if lastLogin.Valid {
		u.LastLogin = lastLogin.Time
	}
	return &u, nil
}

func (c *DB) LoadAccountByRefreshToken(refreshToken string) (*GlobalCommonTypes.AccountRecord, error) {
	query := `
	SELECT u.user_id, u.user_name, u.email, u.password_hash, u.created_at, u.revoked, u.last_login
	FROM users u
	JOIN tokens t ON u.user_id = t.user_id
	WHERE t.id = $1;
	`
	row := c.QueryRow(query, refreshToken)

	var u GlobalCommonTypes.AccountRecord
	var lastLogin sql.NullTime
	err := row.Scan(
		&u.UserID,
		&u.Username,
		&u.Email,
		&u.PasswordHash,
		&u.CreatedAt,
		&u.Revoked,
		&lastLogin,
	)
	if err != nil {
		return nil, fmt.Errorf("failed to load user by refresh token: %w", err)
	}
	if lastLogin.Valid {
		u.LastLogin = lastLogin.Time
	}
	return &u, nil
}

func (c *DB) UpdateLastLogin(userID string, lastLoginTime sql.NullTime) error {
	query := `UPDATE users SET last_login = $1 WHERE user_id = $2;`
	_, err := c.Exec(query, lastLoginTime, userID)
	if err != nil {
		return fmt.Errorf("failed to update last login: %w", err)
	}
	return nil
}

func (c *DB) RevokeAccount(userID string) error {
	query := `UPDATE users SET revoked = TRUE WHERE user_id = $1;`
	_, err := c.Exec(query, userID)
	if err != nil {
		return fmt.Errorf("failed to revoke account: %w", err)
	}
	return nil
}

func (c *DB) ActivateAccount(userID string) error {
	query := `UPDATE users SET revoked = FALSE WHERE user_id = $1;`
	_, err := c.Exec(query, userID)
	if err != nil {
		return fmt.Errorf("failed to unrevoke account: %w", err)
	}
	return nil
}

func (c *DB) DeleteAccount(userID string) error {
	query := `DELETE FROM users WHERE user_id = $1;`
	_, err := c.Exec(query, userID)
	if err != nil {
		return fmt.Errorf("failed to delete account: %w", err)
	}
	return nil
}

func (c *DB) UpdatePassword(userID, newPasswordHash string) error {
	query := `UPDATE users SET password_hash = $1 WHERE user_id = $2;`
	_, err := c.Exec(query, newPasswordHash, userID)
	if err != nil {
		return fmt.Errorf("failed to update password: %w", err)
	}
	return nil
}

func (c *DB) UpdateUsername(userID, newUsername string) error {
	query := `UPDATE users SET user_name = $1 WHERE user_id = $2;`
	_, err := c.Exec(query, newUsername, userID)
	if err != nil {
		return fmt.Errorf("failed to update username: %w", err)
	}
	return nil
}
