package JWT_Tokens

import (
	"time"
)

type JWTConfig struct {
	SessionDurationMinutes int `json:"SessionDurationMinutes"`
	LongTimeDurationDays   int `json:"RefreshDurationDays"`
	RefreshWindowMinutes   int `json:"RefreshWindowMinutes"`
}

func (c *JWTConfig) SessionDuration() time.Duration {
	return time.Duration(c.SessionDurationMinutes) * time.Minute
}

func (c *JWTConfig) RefreshDuration() time.Duration {
	return time.Duration(c.LongTimeDurationDays) * 24 * time.Hour
}

func (c *JWTConfig) RefreshWindow() time.Duration {
	return time.Duration(c.RefreshWindowMinutes) * time.Minute
}
