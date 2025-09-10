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

func (c *VmAuthClient) NewSessionToken(ctx context.Context, refreshToken string) (string, error) {
	req := &pb.NewSessionTokenRequest{
		RefreshToken: refreshToken,
	}
	res, err := c.Client.NewSessionToken(ctx, req)
	if err != nil {
		return "", err
	}
	return res.SessionToken, nil
}

func (c *VmAuthClient) RevokeToken(ctx context.Context, token, userID string) error {
	req := &pb.RevokeTokenRequest{
		Token:  token,
		UserID: userID,
	}
	_, err := c.Client.RevokeToken(ctx, req)
	if err != nil {
		return err
	}
	return nil
}

func (c *VmAuthClient) ActivateToken(ctx context.Context, token, userID string) error {
	req := &pb.ActivateTokenRequest{
		Token:  token,
		UserID: userID,
	}
	_, err := c.Client.ActivateToken(ctx, req)
	if err != nil {
		return err
	}
	return nil
}
