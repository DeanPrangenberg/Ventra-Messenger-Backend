package PayloadUtils

import (
	"context"
	"encoding/json"
	"log"
	"time"
)

func ParsePayload(payload json.RawMessage, target interface{}) error {
	if err := json.Unmarshal(payload, target); err != nil {
		log.Printf("[ERROR] Failed to parse payload: %v", err)
		return err
	}
	return nil
}

func WithTimeoutContext(duration time.Duration) (context.Context, context.CancelFunc) {
	return context.WithTimeout(context.Background(), duration)
}
