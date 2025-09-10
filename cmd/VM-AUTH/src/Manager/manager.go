package Manager

import (
	PostgresWrapper "PostgresWrapper/src"
	RedisWrapper "RedisWrapper/src"
	"VM-AUTH/src/AuthCommonTypes"
	"VM-AUTH/src/JWT-Tokens"
	"sync"

	"log"
)

var (
	connections = make(map[string]*AuthCommonTypes.UserSession)
	connMutex   sync.Mutex
)

var (
	db      PostgresWrapper.DB
	dbMutex sync.Mutex
)

var (
	redisClient      RedisWrapper.Client
	redisClientMutex sync.Mutex
)

// singleton JWT manager
var (
	jwtManager *JWT_Tokens.TokenManager
	jwtMutex   sync.Mutex
)

func GetDB() *PostgresWrapper.DB {
	dbMutex.Lock()
	defer dbMutex.Unlock()

	if !db.Connected {
		//TODO: Load actual Postgres connection details farom kubernetes secret
		var dbPtr, err = PostgresWrapper.Connect("localhost", "5432", "postgres", "postgres", "postgres")
		if err != nil {
			log.Fatalf("Failed to connect to database: %v", err)
		}
		db = *dbPtr
		log.Println("Database connection established")
	}

	return &db
}

func GetJwtManager() *JWT_Tokens.TokenManager {
	jwtMutex.Lock()
	defer jwtMutex.Unlock()

	if jwtManager != nil {
		return jwtManager
	}

	config := JWT_Tokens.JWTConfig{
		10,
		14,
		5,
	}
	jm, err := JWT_Tokens.NewTokenManager(&config, GetDB())
	if err != nil {
		log.Fatalf("Failed to create JWT manager: %v", err)
	}
	jwtManager = jm
	return jwtManager
}
