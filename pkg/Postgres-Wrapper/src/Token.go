package PostgresWrapper

import (
	"GlobalCommonTypes"
	"fmt"
)

func (c *DB) AddToken(data *GlobalCommonTypes.TokenRecord) error {
	query := `
	INSERT INTO tokens (id, user_id, token_type, created_at, expires_at, parent_token, revoked, last_used_at)
	VALUES ($1, $2, $3, $4, $5, $6, $7, $8);`
	_, err := c.Exec(query,
		data.ID,
		data.UserID,
		data.TokenType,
		data.CreatedAt,
		data.ExpiresAt,
		data.ParentToken,
		data.Revoked,
		data.LastUsedAt,
	)

	if err != nil {
		return fmt.Errorf("failed to create token tables: %w", err)
	}
	return nil
}

func (c *DB) DeleteToken(ID string) error {
	query := `DELETE FROM tokens WHERE id = $1;`
	_, err := c.Exec(query,
		ID,
	)

	if err != nil {
		return fmt.Errorf("failed to delete token: %w", err)
	}
	return nil
}

func (c *DB) DeleteExpiredTokens() error {
	query := `DELETE FROM tokens WHERE expires_at < NOW();`
	_, err := c.Exec(query)
	if err != nil {
		return fmt.Errorf("failed to clean expired tokens: %w", err)
	}
	return nil
}

func (c *DB) DeleteRevokedTokens() error {
	query := `DELETE FROM tokens WHERE revoked = $1;`
	_, err := c.Exec(query,
		true,
	)

	if err != nil {
		return fmt.Errorf("failed to clean expired tokens: %w", err)
	}

	return nil
}

func (c *DB) LoadToken(ID string) (*GlobalCommonTypes.TokenRecord, error) {
	query := `SELECT id, user_id, token_type, created_at, expires_at, parent_token, revoked, last_used_at FROM tokens WHERE id = $1;`
	row := c.QueryRow(query, ID)

	var token GlobalCommonTypes.TokenRecord
	err := row.Scan(
		&token.ID,
		&token.UserID,
		&token.TokenType,
		&token.CreatedAt,
		&token.ExpiresAt,
		&token.ParentToken,
		&token.Revoked,
		&token.LastUsedAt,
	)
	if err != nil {
		return nil, fmt.Errorf("failed to load token: %w", err)
	}
	return &token, nil
}

// LoadTokensByUser returns all tokens belonging to a specific user.
func (c *DB) LoadTokensByUser(userID string) ([]GlobalCommonTypes.TokenRecord, error) {
	query := `SELECT id, user_id, token_type, created_at, expires_at, parent_token, revoked, last_used_at FROM tokens WHERE user_id = $1;`
	rows, err := c.Query(query, userID)
	if err != nil {
		return nil, fmt.Errorf("failed to query tokens by user: %w", err)
	}
	defer rows.Close()

	tokens := make([]GlobalCommonTypes.TokenRecord, 0)
	for rows.Next() {
		var t GlobalCommonTypes.TokenRecord
		if err := rows.Scan(
			&t.ID,
			&t.UserID,
			&t.TokenType,
			&t.CreatedAt,
			&t.ExpiresAt,
			&t.ParentToken,
			&t.Revoked,
			&t.LastUsedAt,
		); err != nil {
			return nil, fmt.Errorf("failed to scan token row: %w", err)
		}
		tokens = append(tokens, t)
	}
	if err := rows.Err(); err != nil {
		return nil, fmt.Errorf("rows error: %w", err)
	}
	return tokens, nil
}

func (c *DB) RevokeToken(ID string) error {
	// Set revoked to true
	updateQuery := `UPDATE tokens SET revoked = true WHERE id = $1;`
	_, err := c.Exec(updateQuery, ID)
	if err != nil {
		return fmt.Errorf("failed to revoke token: %w", err)
	}

	return nil
}

func (c *DB) ActivateToken(ID string) error {
	// Set revoked to true
	updateQuery := `UPDATE tokens SET revoked = false WHERE id = $1;`
	_, err := c.Exec(updateQuery, ID)
	if err != nil {
		return fmt.Errorf("failed to revoke token: %w", err)
	}

	return nil
}
