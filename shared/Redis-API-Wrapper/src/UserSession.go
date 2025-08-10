package RedisWrapper

import (
	gRPC "Redis-API-Wrapper/Redis-API-gRPC"
	"fmt"
)

func (c *Client) SetUserSession(userID, apiID string) error {
	req := &gRPC.SetUserSessionRequest{
		UserId: userID,
		Api:    apiID,
	}

	resp, err := c.Client.SetUserSession(c.Ctx, req)

	if err != nil {
		return fmt.Errorf("failed to set api session %s for user %s: %w", apiID, userID, err)
	}

	if resp == nil {
		return fmt.Errorf("received nil response when setting api session %s for user %s", apiID, userID)
	}

	if !resp.Success {
		if resp.Message != "" {
			return fmt.Errorf("failed to set api session %s for user %s: %s", apiID, userID, resp.Message)
		}
		return fmt.Errorf("failed to set api session %s for user %s: unknown error", apiID, userID)
	}

	if c.DebugPrints {
		fmt.Println(resp.Message)
	}

	return nil
}

func (c *Client) GetUserSession(userID string) (string, error) {
	req := &gRPC.GetUserSessionRequest{
		UserId: userID,
	}

	resp, err := c.Client.GetUserSession(c.Ctx, req)

	if err != nil {
		return "", fmt.Errorf("failed to get api session for user %s: %w", userID, err)
	}

	if resp == nil {
		return "", fmt.Errorf("received nil response when getting api session for user %s", userID)
	}

	if c.DebugPrints {
		fmt.Println(resp.Api)
	}

	return resp.Api, nil
}
