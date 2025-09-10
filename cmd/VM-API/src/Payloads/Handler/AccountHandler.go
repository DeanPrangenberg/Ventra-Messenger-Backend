package Handler

import (
	"VM-API/src/ApiCommonTypes"
	"VM-API/src/ConnectionManager"
	"VM-API/src/Payloads/PayloadUtils"
	"context"
	"time"
)

func LoginHandler(session *ApiCommonTypes.WebSocketSession, payload ApiCommonTypes.PayloadSkeleton) error {
	return HandleWithTimeout(session, payload, handleLogin, 10*time.Second)
}

func handleLogin(ctx context.Context, session *ApiCommonTypes.WebSocketSession, payload ApiCommonTypes.PayloadSkeleton) error {
	AC := ConnectionManager.GetAuthClient()

	var loginReq ApiCommonTypes.LoginRequest
	if err := PayloadUtils.ParsePayload(payload.InternalPayload, &loginReq); err != nil {
		return PayloadUtils.SendErrorResponse(session, payload.RequestID, "Invalid login payload")
	}

	refreshToken, sessionToken, userID, err := AC.Login(ctx, loginReq.Email, loginReq.Password)
	if err != nil {
		return PayloadUtils.SendErrorResponse(session, payload.RequestID, "Login failed: "+err.Error())
	}

	response := ApiCommonTypes.LoginResponse{
		RefreshToken: refreshToken,
		SessionToken: sessionToken,
		UserID:       userID,
	}

	return PayloadUtils.SendEncryptedResponse(session, payload.RequestID, "loginResponse", response)
}

func RegisterHandler(session *ApiCommonTypes.WebSocketSession, payload ApiCommonTypes.PayloadSkeleton) error {
	return HandleWithTimeout(session, payload, handleRegister, 10*time.Second)
}

func handleRegister(ctx context.Context, session *ApiCommonTypes.WebSocketSession, payload ApiCommonTypes.PayloadSkeleton) error {
	AC := ConnectionManager.GetAuthClient()

	var loginReq ApiCommonTypes.RegisterRequest
	if err := PayloadUtils.ParsePayload(payload.InternalPayload, &loginReq); err != nil {
		return PayloadUtils.SendErrorResponse(session, payload.RequestID, "Invalid register payload")
	}

	refreshToken, sessionToken, userID, err := AC.Register(ctx, loginReq.Username, loginReq.Email, loginReq.Password)
	if err != nil {
		return PayloadUtils.SendErrorResponse(session, payload.RequestID, "Register failed: "+err.Error())
	}

	response := ApiCommonTypes.RegisterResponse{
		RefreshToken: refreshToken,
		SessionToken: sessionToken,
		UserID:       userID,
	}

	return PayloadUtils.SendEncryptedResponse(session, payload.RequestID, "RegisterResponse", response)
}

func UpdatePasswordHandler(session *ApiCommonTypes.WebSocketSession, payload ApiCommonTypes.PayloadSkeleton) error {
	return HandleWithTimeout(session, payload, handleUpdatePassword, 10*time.Second)
}

func handleUpdatePassword(ctx context.Context, session *ApiCommonTypes.WebSocketSession, payload ApiCommonTypes.PayloadSkeleton) error {
	AC := ConnectionManager.GetAuthClient()

	var loginReq ApiCommonTypes.UpdatePasswordRequest
	if err := PayloadUtils.ParsePayload(payload.InternalPayload, &loginReq); err != nil {
		return PayloadUtils.SendErrorResponse(session, payload.RequestID, "Invalid Password update failed payload")
	}

	err := AC.UpdatePassword(ctx, loginReq.OldPassword, loginReq.RefreshToken, loginReq.NewPassword)
	if err != nil {
		return PayloadUtils.SendErrorResponse(session, payload.RequestID, "Password update failed: "+err.Error())
	}

	response := ApiCommonTypes.UpdatePasswordResponse{}

	return PayloadUtils.SendEncryptedResponse(session, payload.RequestID, "UpdatePasswordResponse", response)
}

func UpdateUsernameHandler(session *ApiCommonTypes.WebSocketSession, payload ApiCommonTypes.PayloadSkeleton) error {
	return HandleWithTimeout(session, payload, handleUpdateUsername, 10*time.Second)
}

func handleUpdateUsername(ctx context.Context, session *ApiCommonTypes.WebSocketSession, payload ApiCommonTypes.PayloadSkeleton) error {
	AC := ConnectionManager.GetAuthClient()

	var loginReq ApiCommonTypes.UpdateUsernameRequest
	if err := PayloadUtils.ParsePayload(payload.InternalPayload, &loginReq); err != nil {
		return PayloadUtils.SendErrorResponse(session, payload.RequestID, "Invalid Username update failed payload")
	}

	err := AC.UpdateUsername(ctx, loginReq.OldPassword, loginReq.RefreshToken, loginReq.NewUsername)
	if err != nil {
		return PayloadUtils.SendErrorResponse(session, payload.RequestID, "Username update failed: "+err.Error())
	}

	response := ApiCommonTypes.UpdateUsernameResponse{}

	return PayloadUtils.SendEncryptedResponse(session, payload.RequestID, "UpdateUsernameResponse", response)
}

func DeleteAccountHandler(session *ApiCommonTypes.WebSocketSession, payload ApiCommonTypes.PayloadSkeleton) error {
	return HandleWithTimeout(session, payload, handleDeleteAccount, 10*time.Second)
}

func handleDeleteAccount(ctx context.Context, session *ApiCommonTypes.WebSocketSession, payload ApiCommonTypes.PayloadSkeleton) error {
	AC := ConnectionManager.GetAuthClient()

	var loginReq ApiCommonTypes.DeleteAccountRequest
	if err := PayloadUtils.ParsePayload(payload.InternalPayload, &loginReq); err != nil {
		return PayloadUtils.SendErrorResponse(session, payload.RequestID, "Invalid DeleteAccount payload")
	}

	err := AC.DeleteAccount(ctx, loginReq.RefreshToken, loginReq.Password)
	if err != nil {
		return PayloadUtils.SendErrorResponse(session, payload.RequestID, "DeleteAccount failed: "+err.Error())
	}

	response := ApiCommonTypes.DeleteAccountResponse{}

	return PayloadUtils.SendEncryptedResponse(session, payload.RequestID, "DeleteAccountResponse", response)
}
