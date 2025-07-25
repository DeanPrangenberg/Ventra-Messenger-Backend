package RedisWrapper

import (
	Redis_gRPC "Redis-API-Wrapper/Redis-API-gRPC"
	"context"
	"google.golang.org/grpc"
)

// New erstellt einen neuen gRPC-Client
func New(address string, debugPrints bool) (*Client, error) {
	// Für Produktion: grpc.WithTransportCredentials(credentials.NewTLS(...))
	conn, err := grpc.NewClient(address, grpc.WithInsecure())
	if err != nil {
		return nil, err
	}

	client := Redis_gRPC.NewUserStatusServiceClient(conn)

	return &Client{
		conn:        conn,
		Client:      client,
		Ctx:         context.Background(),
		DebugPrints: debugPrints,
	}, nil
}

func (c *Client) Close() error {
	return c.conn.Close()
}
