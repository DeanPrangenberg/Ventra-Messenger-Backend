package Manager

import (
	PostgresWrapper "PostgresWrapper/src"
	RedisWrapper "RedisWrapper/src"
	"VM-AUTH/src/AuthCommonTypes"
	"VM-AUTH/src/JWT-Tokens"
	"VM-AUTH/src/PrometheusEndpoint"
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

func AddConnection(uuid string, conn *AuthCommonTypes.UserSession) {
	connMutex.Lock()
	defer connMutex.Unlock()
	connections[uuid] = conn
	PrometheusEndpoint.ConnectedClients.Inc()
	log.Println(uuid, " added. Total connections:", len(connections))
}

func RemoveConnection(uuid string) {
	connMutex.Lock()
	defer connMutex.Unlock()
	delete(connections, uuid)
	PrometheusEndpoint.ConnectedClients.Dec()
	log.Println(uuid, " removed. Total connections:", len(connections))
}

func GetConnection(uuid string) (*AuthCommonTypes.UserSession, bool) {
	connMutex.Lock()
	defer connMutex.Unlock()
	conn, exists := connections[uuid]
	return conn, exists
}

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

func GetRedisApi() *RedisWrapper.Client {
	redisClientMutex.Lock()
	defer redisClientMutex.Unlock()
	err := redisClient.Connected()
	if err != nil {
		redisClient = *RedisWrapper.New("localhost:6379", true)
		log.Println("Redis connection established")
	}

	return &redisClient
}
