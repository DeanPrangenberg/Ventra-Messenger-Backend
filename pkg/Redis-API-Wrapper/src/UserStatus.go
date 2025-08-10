package RedisWrapper

import (
	gRPC "Redis-API-Wrapper/Redis-API-gRPC"
	"fmt"
)

func (c *Client) SetUserStatus(userID, status string) error {
	req := &gRPC.UserStatusRequest{
		UserId: userID,
		Status: status,
	}

	resp, err := c.Client.SetUserStatus(c.Ctx, req)

	if err != nil {
		return fmt.Errorf("failed to set status session %s for user %s: %w", status, userID, err)
	}

	if resp == nil {
		return fmt.Errorf("received nil response when setting status session %s for user %s", status, userID)
	}

	if !resp.Success {
		if resp.Message != "" {
			return fmt.Errorf("failed to set status session %s for user %s: %s", status, userID, resp.Message)
		}
		return fmt.Errorf("failed to set status session %s for user %s: unknown error", status, userID)
	}

	if c.DebugPrints {
		fmt.Println(resp.Message)
	}

	return nil
}

func (c *Client) GetUserStatus(userID string) (string, error) {
	req := &gRPC.UserID{
		UserId: userID,
	}

	resp, err := c.Client.GetUserStatus(c.Ctx, req)

	if err != nil {
		return "", fmt.Errorf("failed to get api session for user %s: %w", userID, err)
	}

	if resp == nil {
		return "", fmt.Errorf("received nil response when getting api session for user %s", userID)
	}

	if c.DebugPrints {
		fmt.Println(resp.Status)
	}

	return resp.Status, nil
}
