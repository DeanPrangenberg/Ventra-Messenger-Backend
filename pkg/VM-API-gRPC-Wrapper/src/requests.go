package VM_API_gRPC_Wrapper

import (
	pb "VM-API-gRPC-Wrapper/gen-pb"
	"context"
)

func (c *VmApiClient) IsUserConnected(userID string) (bool, error) {
	req := &pb.UserStatusRequest{UserId: userID}
	resp, err := c.Client.IsUserConnected(context.Background(), req)
	if err != nil {
		return false, err
	}
	return resp.Connected, nil
}

func (c *VmApiClient) SendPayload(userID string, payload []byte) (string, error) {
	req := &pb.PayloadRequest{UserId: userID, Payload: payload}
	resp, err := c.Client.SendPayload(context.Background(), req)
	if err != nil {
		return "", err
	}
	return resp.Ack, nil
}
