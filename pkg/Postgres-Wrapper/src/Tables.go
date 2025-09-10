package PostgresWrapper

import (
	"fmt"
)

func (c *DB) CreateTokenTables() error {
	queries := []string{
		`CREATE TABLE IF NOT EXISTS tokens (
			id VARCHAR(255) PRIMARY KEY,
			user_id VARCHAR(255) NOT NULL,
			token_type VARCHAR(50) NOT NULL,
			created_at TIMESTAMP NOT NULL,
			expires_at TIMESTAMP NOT NULL,
			parent_token VARCHAR(255),
			revoked BOOLEAN DEFAULT FALSE,
			last_used_at TIMESTAMP NOT NULL,
			FOREIGN KEY (parent_token) REFERENCES tokens(id),
			FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE
		);`,
		`CREATE TABLE IF NOT EXISTS jwt_keys (
			id SERIAL PRIMARY KEY,
			private_key TEXT NOT NULL,
			public_key TEXT NOT NULL,
			active BOOLEAN DEFAULT TRUE,
			created_at TIMESTAMP NOT NULL DEFAULT NOW()
		);`,
	}

	for _, query := range queries {
		if _, err := c.Exec(query); err != nil {
			return fmt.Errorf("failed to create token tables: %w", err)
		}
	}
	return nil
}

func (c *DB) CreateUserTables() error {
	query := `
	CREATE TABLE IF NOT EXISTS users (
	    user_id VARCHAR(255) PRIMARY KEY,
	    user_name VARCHAR(255) NOT NULL,
	    email VARCHAR(255) UNIQUE NOT NULL,
	    password_hash TEXT NOT NULL,
	    created_at TIMESTAMP NOT NULL,
	    revoked BOOLEAN DEFAULT FALSE,
	    last_login TIMESTAMP
	);
	`
	_, err := c.Exec(query)
	if err != nil {
		return fmt.Errorf("failed to create user tables: %w", err)
	}
	return nil
}
