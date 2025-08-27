package VM_API_gRPC_Wrapper

import (
	pb "VM-API-gRPC-Wrapper/gen-pb"

	"google.golang.org/grpc"
)

type VmApiClient struct {
	Conn   *grpc.ClientConn
	Client pb.UserApiClient
}

func NewVmApiClient(address string) (*VmApiClient, error) {
	conn, err := grpc.Dial(address, grpc.WithInsecure())
	if err != nil {
		return nil, err
	}
	client := pb.NewUserApiClient(conn)
	return &VmApiClient{Conn: conn, Client: client}, nil
}

func (c *VmApiClient) Close() error {
	return c.Conn.Close()
}
