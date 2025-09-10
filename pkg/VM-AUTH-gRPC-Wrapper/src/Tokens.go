package VM_AUTH_gRPC_Wrapper

import (
	pb "VM-AUTH-gRPC-Wrapper/gen-pb"
	"context"
)

func (c *VmAuthClient) VerifyToken(ctx context.Context, token, userID string) (bool, error) {
	req := &pb.VerifyTokenRequest{
		Token:  token,
		UserID: userID,
	}
	res, err := c.Client.VerifyToken(ctx, req)
	if err != nil {
		return false, err
	}
	return res.Valid, nil
}

func (c *VmAuthClient) RefreshSessionToken(ctx context.Context, refreshToken string) (string, error) {
	req := &pb.RefreshSessionTokenRequest{
		RefreshToken: refreshToken,
	}
	res, err := c.Client.RefreshSessionToken(ctx, req)
	if err != nil {
		return "", err
	}
	return res.SessionToken, nil
}
