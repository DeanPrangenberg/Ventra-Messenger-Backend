package RedisWrapper

import (
	"fmt"
)

func (c *Client) SetUserSession(userID, apiID string) error {
	err := c.RedisClient.Set(c.Ctx, "user:session:"+userID, apiID, 0).Err()
	if err != nil {
		return fmt.Errorf("failed to set user session: %w", err)
	}

	if c.DebugPrints {
		fmt.Println("Set UserSession", userID, "to", apiID)
	}

	return nil
}

func (c *Client) GetUserSession(userID string) (string, error) {
	api, err := c.RedisClient.Get(c.Ctx, "user:session:"+userID).Result()
	if err != nil {
		return "", fmt.Errorf("failed to get user session: %w", err)
	}

	if c.DebugPrints {
		fmt.Println(api)
	}

	return api, nil
}
