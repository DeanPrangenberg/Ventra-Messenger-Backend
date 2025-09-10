package Handler

import (
	"VM-API/src/ApiCommonTypes"
	"VM-API/src/ConnectionManager"
	"VM-API/src/Payloads/PayloadUtils"
	"context"
	"time"
)

func NewSessionHandler(session *ApiCommonTypes.WebSocketSession, payload ApiCommonTypes.PayloadSkeleton) error {
	return HandleWithTimeout(session, payload, handleNewSession, 10*time.Second)
}

func handleNewSession(ctx context.Context, session *ApiCommonTypes.WebSocketSession, payload ApiCommonTypes.PayloadSkeleton) error {
	AC := ConnectionManager.GetAuthClient()

	var loginReq ApiCommonTypes.NewSessionTokenRequest
	if err := PayloadUtils.ParsePayload(payload.InternalPayload, &loginReq); err != nil {
		return PayloadUtils.SendErrorResponse(session, payload.RequestID, "Invalid NewSessionToken payload")
	}

	sessionToken, err := AC.NewSessionToken(ctx, loginReq.RefreshToken)
	if err != nil {
		return PayloadUtils.SendErrorResponse(session, payload.RequestID, "NewSessionToken failed: "+err.Error())
	}

	response := ApiCommonTypes.NewSessionTokenResponse{
		SessionToken: sessionToken,
	}

	return PayloadUtils.SendEncryptedResponse(session, payload.RequestID, "NewSessionTokenResponse", response)
}
