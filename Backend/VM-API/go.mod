module VM-API

go 1.24

require (
	github.com/google/uuid v1.6.0
	github.com/gorilla/websocket v1.5.3
	golang.org/x/crypto v0.39.0
	redisAPI v0.0.0-00010101000000-000000000000
)

require (
	github.com/cespare/xxhash/v2 v2.3.0 // indirect
	github.com/dgryski/go-rendezvous v0.0.0-20200823014737-9f7001d12a5f // indirect
	github.com/redis/go-redis/v9 v9.11.0 // indirect
	golang.org/x/sys v0.33.0 // indirect
)

replace redisAPI => ../shared/redisAPI
