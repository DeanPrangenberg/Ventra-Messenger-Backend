package VM_AUTH_gRPC_Wrapper

import (
	pb "VM-AUTH-gRPC-Wrapper/gen-pb"

	"google.golang.org/grpc"
)

type VmAuthClient struct {
	Conn   *grpc.ClientConn
	Client pb.UserAuthClient
}

func NewVmAuthClient(address string) (*VmAuthClient, error) {
	conn, err := grpc.Dial(address, grpc.WithInsecure())
	if err != nil {
		return nil, err
	}
	client := pb.NewUserAuthClient(conn)
	return &VmAuthClient{Conn: conn, Client: client}, nil
}

func (c *VmAuthClient) Close() error {
	return c.Conn.Close()
}
