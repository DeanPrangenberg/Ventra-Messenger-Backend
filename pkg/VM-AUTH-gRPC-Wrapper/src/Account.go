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

func (c *VmAuthClient) UpdatePassword(
	ctx context.Context, oldPassword, refreshToken, newPassword string) error {
	req := &pb.UpdatePasswordRequest{
		OldPassword:  oldPassword,
		RefreshToken: refreshToken,
		NewPassword:  newPassword,
	}
	_, err := c.Client.UpdatePassword(ctx, req)
	if err != nil {
		return err
	}
	return nil
}

func (c *VmAuthClient) UpdateUsername(
	ctx context.Context, oldPassword, refreshToken, newUsername string) error {
	req := &pb.UpdateUsernameRequest{
		OldPassword:  oldPassword,
		RefreshToken: refreshToken,
		NewUsername:  newUsername,
	}
	_, err := c.Client.UpdateUsername(ctx, req)
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

func (c *VmAuthClient) RevokeAccount(ctx context.Context, userid string) error {
	req := &pb.RevokeAccountRequest{
		UserID: userid,
	}
	_, err := c.Client.RevokeAccount(ctx, req)
	if err != nil {
		return err
	}
	return nil
}

func (c *VmAuthClient) ActivateAccount(ctx context.Context, userid string) error {
	req := &pb.ActivateAccountRequest{
		UserID: userid,
	}
	_, err := c.Client.ActivateAccount(ctx, req)
	if err != nil {
		return err
	}
	return nil
}
