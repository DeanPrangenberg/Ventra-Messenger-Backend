package Handler

import (
	"VM-API/src/ApiCommonTypes"
	"VM-API/src/Payloads/PayloadUtils"
	"context"
	"fmt"
	"time"
)

type HandlerFunc func(context.Context, *ApiCommonTypes.WebSocketSession, ApiCommonTypes.PayloadSkeleton) error

func HandleWithTimeout(
	session *ApiCommonTypes.WebSocketSession,
	payload ApiCommonTypes.PayloadSkeleton,
	handler HandlerFunc,
	timeout time.Duration,
) error {
	ctx, cancel := PayloadUtils.WithTimeoutContext(timeout)
	defer cancel()

	errChan := make(chan error, 1)
	go func() {
		errChan <- handler(ctx, session, payload)
	}()

	select {
	case err := <-errChan:
		return err
	case <-ctx.Done():
		return PayloadUtils.SendErrorResponse(session, payload.RequestID, fmt.Sprintf("Request %s timed out", payload.RequestID))
	}
}
