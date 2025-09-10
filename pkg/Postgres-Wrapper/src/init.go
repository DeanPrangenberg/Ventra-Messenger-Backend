package PostgresWrapper

import (
	"database/sql"
	"fmt"

	_ "github.com/lib/pq"
)

type DB struct {
	*sql.DB
	Connected bool
}

// Connect erstellt eine neue Datenbankverbindung
func Connect(host, port, user, password, dbname string) (*DB, error) {
	psqlInfo := fmt.Sprintf("host=%s port=%s user=%s password=%s dbname=%s sslmode=disable",
		host, port, user, password, dbname)

	db, err := sql.Open("postgres", psqlInfo)
	if err != nil {
		return nil, err
	}

	if err := db.Ping(); err != nil {
		return nil, err
	}

	DB := &DB{
		DB:        db,
		Connected: true,
	}

	// Create user tables first, tokens reference users
	err = DB.CreateUserTables()
	if err != nil {
		return nil, err
	}

	err = DB.CreateTokenTables()
	if err != nil {
		return nil, err
	}

	return DB, nil
}
