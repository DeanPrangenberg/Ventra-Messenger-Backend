package gRPCserver

import (
	"GlobalCommonTypes"
	pb "VM-AUTH-gRPC-Wrapper/gen-pb"
	"VM-AUTH/src/Manager"
	"VM-AUTH/src/PrometheusEndpoint/PrometheusCounters"
	"context"
	"errors"
	"log"
	"time"

	"github.com/google/uuid"
)

func (s *VMAuthServer) Login(ctx context.Context, req *pb.LoginRequest) (*pb.LoginResponse, error) {
	log.Printf("[INFO] Login attempt for email: %s", req.Email)
	DB := Manager.GetDB()
	JM := Manager.GetJwtManager()

	PrometheusCounters.LoginAttemptsTotal.Inc()

	LoadedUser, err := DB.LoadAccountByEmail(req.Email)
	if err != nil {
		PrometheusCounters.LoginFailures.Inc()
		log.Printf("[WARN] Login failed for email %s: %v", req.Email, err)
		return nil, errors.New("invalid email or password")
	}

	// Verify the password
	if !(req.Password == LoadedUser.PasswordHash) {
		PrometheusCounters.LoginFailures.Inc()
		log.Printf("[WARN] Login failed for email %s: incorrect password", req.Email)
		return nil, errors.New("invalid email or password")
	}

	refreshToken, err := JM.CreateRefreshToken(LoadedUser.UserID)
	if err != nil {
		return nil, err
	}

	sessionToken, err := JM.CreateSessionToken(refreshToken)
	if err != nil {
		return nil, err
	}

	rep := &pb.LoginResponse{
		UserID:       LoadedUser.UserID,
		RefreshToken: refreshToken,
		SessionToken: sessionToken,
	}

	// Increment the successful login metric
	PrometheusCounters.LoginSuccess.Inc()
	log.Printf("[INFO] User %s logged in successfully", LoadedUser.UserID)

	return rep, nil
}

func (s *VMAuthServer) Register(ctx context.Context, req *pb.RegisterRequest) (*pb.RegisterResponse, error) {
	log.Printf("[INFO] Register attempt for email: %s", req.Email)
	DB := Manager.GetDB()
	JM := Manager.GetJwtManager()

	PrometheusCounters.RegisterAttemptsTotal.Inc()

	// Check if the user already exists
	_, err := DB.LoadAccountByID(req.Email)
	if err == nil {
		PrometheusCounters.RegisterFailures.Inc()
		log.Printf("[WARN] Registration failed for email %s: user already exists", req.Email)
		return nil, errors.New("user already exists")
	}

	userRecord := GlobalCommonTypes.AccountRecord{
		UserID:       uuid.New().String(),
		Username:     req.Username,
		Email:        req.Email,
		PasswordHash: req.Password,
		CreatedAt:    time.Now(),
		Revoked:      false,
	}

	err = DB.AddAccount(&userRecord)
	if err != nil {
		PrometheusCounters.RegisterFailures.Inc()
		log.Printf("[ERROR] Failed to register user %s: %v", req.Email, err)
		return nil, err
	}

	longTimeToken, err := JM.CreateRefreshToken(userRecord.UserID)
	if err != nil {
		return nil, err
	}

	sessionToken, err := JM.CreateSessionToken(longTimeToken)
	if err != nil {
		return nil, err
	}

	rep := &pb.RegisterResponse{
		UserID:       userRecord.UserID,
		RefreshToken: longTimeToken,
		SessionToken: sessionToken,
	}

	// Increment the successful registration metric
	PrometheusCounters.RegisterSuccess.Inc()
	log.Printf("[INFO] User %s registered successfully", userRecord.UserID)

	return rep, nil
}

func (s *VMAuthServer) UpdateUsername(ctx context.Context, req *pb.UpdateUsernameRequest) (*pb.UpdateUsernameResponse, error) {
	log.Printf("[INFO] Updating username for Account with RefreshToken: %s", req.RefreshToken)
	DB := Manager.GetDB()

	PrometheusCounters.UpdateUsernameAttemptsTotal.Inc()

	LoadedUser, err := DB.LoadAccountByRefreshToken(req.RefreshToken)
	if err != nil {
		PrometheusCounters.UpdateUsernameFailures.Inc()
		log.Printf("[WARN] Failed to update username for Account with RefreshToken: %s: %v", req.RefreshToken, err)
		return nil, errors.New("invalid email or password")
	}

	// Verify the password
	if !(req.OldPassword == LoadedUser.PasswordHash) {
		PrometheusCounters.UpdateUsernameFailures.Inc()
		log.Printf("[WARN] Failed to update username for Account with RefreshToken: %s: wrong Password", req.RefreshToken)
		return nil, errors.New("invalid email or password")
	}

	err = DB.UpdateUsername(LoadedUser.UserID, req.OldPassword)
	if err != nil {
		PrometheusCounters.UpdateUsernameFailures.Inc()
		log.Printf("[WARN] Failed to update username for Account with RefreshToken: %s: %v", req.RefreshToken, err)
		return nil, err
	}

	// Increment the successful login metric
	PrometheusCounters.UpdateUsernameSuccess.Inc()
	log.Printf("[INFO] Updated username for Account with RefreshToken: %s", req.RefreshToken)

	return &pb.UpdateUsernameResponse{}, nil
}

func (s *VMAuthServer) UpdatePassword(ctx context.Context, req *pb.UpdatePasswordRequest) (*pb.UpdatePasswordResponse, error) {
	log.Printf("[INFO] Updating username for Account with RefreshToken: %s", req.RefreshToken)
	DB := Manager.GetDB()

	PrometheusCounters.UpdatePasswordAttemptsTotal.Inc()

	LoadedUser, err := DB.LoadAccountByRefreshToken(req.RefreshToken)
	if err != nil {
		PrometheusCounters.UpdatePasswordFailures.Inc()
		log.Printf("[WARN] Failed to update username for Account with RefreshToken: %s: %v", req.RefreshToken, err)
		return nil, errors.New("invalid email or password")
	}

	// Verify the password
	if !(req.OldPassword == LoadedUser.PasswordHash) {
		PrometheusCounters.UpdatePasswordFailures.Inc()
		log.Printf("[WARN] Failed to update username for Account with RefreshToken: %s: wrong Password", req.RefreshToken)
		return nil, errors.New("invalid email or password")
	}

	err = DB.UpdatePassword(LoadedUser.UserID, req.OldPassword)
	if err != nil {
		PrometheusCounters.UpdatePasswordFailures.Inc()
		log.Printf("[WARN] Failed to update username for Account with RefreshToken: %s: %v", req.RefreshToken, err)
		return nil, err
	}

	// Increment the successful login metric
	PrometheusCounters.UpdatePasswordSuccess.Inc()
	log.Printf("[INFO] Updated username for Account with RefreshToken: %s", req.RefreshToken)

	return &pb.UpdatePasswordResponse{}, nil
}

func (s *VMAuthServer) DeleteAccount(ctx context.Context, req *pb.DeleteAccountRequest) (*pb.DeleteAccountResponse, error) {
	log.Printf("[INFO] Deleting Account with RefreshToken: %s", req.RefreshToken)
	DB := Manager.GetDB()

	PrometheusCounters.DeleteAccountAttemptsTotal.Inc()

	LoadedUser, err := DB.LoadAccountByRefreshToken(req.RefreshToken)
	if err != nil {
		PrometheusCounters.DeleteAccountFailures.Inc()
		log.Printf("[WARN] Failed to delete Account with RefreshToken: %s: %v", req.RefreshToken, err)
		return nil, errors.New("invalid email or password")
	}

	// Verify the password
	if !(req.Password == LoadedUser.PasswordHash) {
		PrometheusCounters.DeleteAccountFailures.Inc()
		log.Printf("[WARN] Failed to delete Account with RefreshToken: %s: wrong Password", req.RefreshToken)
		return nil, errors.New("invalid email or password")
	}

	err = DB.DeleteAccount(LoadedUser.UserID)
	if err != nil {
		PrometheusCounters.DeleteAccountFailures.Inc()
		log.Printf("[WARN] Failed to delete Account with RefreshToken: %s: %v", req.RefreshToken, err)
		return nil, err
	}

	// Increment the successful login metric
	PrometheusCounters.DeleteAccountSuccess.Inc()
	log.Printf("[INFO] Deleted Account with RefreshToken: %s", req.RefreshToken)

	return &pb.DeleteAccountResponse{}, nil
}
