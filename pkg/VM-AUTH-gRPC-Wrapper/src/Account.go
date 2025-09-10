package VM_AUTH_gRPC_Wrapper

import (
	pb "VM-AUTH-gRPC-Wrapper/gen-pb"
	"context"
)

func (c *VmAuthClient) Login(ctx context.Context, email, password string) (string, string, string, error) {
	req := &pb.LoginRequest{
		Email:    email,
		Password: password,
	}
	res, err := c.Client.Login(ctx, req)
	if err != nil {
		return "", "", "", err
	}
	return res.RefreshToken, res.SessionToken, res.UserID, nil
}

func (c *VmAuthClient) Register(ctx context.Context, Username, email, password string) (string, string, string, error) {
	req := &pb.RegisterRequest{
		Username: Username,
		Email:    email,
		Password: password,
	}
	res, err := c.Client.Register(ctx, req)
	if err != nil {
		return "", "", "", err
	}
	return res.RefreshToken, res.SessionToken, res.UserID, nil
}

func (c *VmAuthClient) UpdateAccount(
	ctx context.Context, oldPassword, refreshToken, newUsername, newPassword string) error {
	req := &pb.UpdateAccountRequest{
		OldPassword:  oldPassword,
		RefreshToken: newPassword,
		NewUsername:  refreshToken,
		NewPassword:  newUsername,
	}
	_, err := c.Client.UpdateAccount(ctx, req)
	if err != nil {
		return err
	}
	return nil
}

func (c *VmAuthClient) DeleteAccount(ctx context.Context, password, refreshToken string) error {
	req := &pb.DeleteAccountRequest{
		Password:     password,
		RefreshToken: refreshToken,
	}
	_, err := c.Client.DeleteAccount(ctx, req)
	if err != nil {
		return err
	}
	return nil
}
