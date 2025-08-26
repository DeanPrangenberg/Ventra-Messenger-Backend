package RedisWrapper

import (
	"fmt"
)

func (c *Client) SetUserStatus(userID, status string) error {
	err := c.RedisClient.Set(c.Ctx, "user:status:"+userID, status, 0).Err()
	if err != nil {
		return fmt.Errorf("failed to set user status: %w", err)
	}

	if c.DebugPrints {
		fmt.Println("Set UserStatus", userID, "to", status)
	}

	return nil
}

func (c *Client) GetUserStatus(userID string) (string, error) {
	status, err := c.RedisClient.Get(c.Ctx, "user:status:"+userID).Result()
	if err != nil {
		return "", fmt.Errorf("failed to get user status: %w", err)
	}

	if c.DebugPrints {
		fmt.Println(status)
	}

	return status, nil
}
