package gRPCserver

import (
	pb "VM-AUTH-gRPC-Wrapper/gen-pb"
	"VM-AUTH/src/Manager"
	"VM-AUTH/src/PrometheusEndpoint/PrometheusCounters"
	"context"
	"errors"
	"log"
	"time"
)

func (s *VMAuthServer) VerifyToken(ctx context.Context, req *pb.VerifyTokenRequest) (*pb.VerifyTokenResponse, error) {
	log.Printf("[INFO] Verifing Token for user: %s", req.UserID)
	JM := Manager.GetJwtManager()
	PrometheusCounters.VerifyTokenTotal.Inc()

	// Verify the token
	claims, err := JM.VerifyToken(req.Token)
	if err != nil {
		PrometheusCounters.VerifyTokenFailures.Inc()
		return nil, err
	}

	// Increment the appropriate metric
	if claims.UserID == req.UserID {
		PrometheusCounters.VerifyTokenSuccess.Inc()
		log.Printf("[INFO] Token verification successful for user %s", req.UserID)
		return &pb.VerifyTokenResponse{Valid: true}, nil
	} else {
		PrometheusCounters.VerifyTokenFailures.Inc()
		log.Printf("[WARN] Token verification failed for user %s: user ID mismatch", req.UserID)
		return &pb.VerifyTokenResponse{Valid: false}, nil
	}
}

func (s *VMAuthServer) RevokeToken(ctx context.Context, req *pb.RevokeTokenRequest) (*pb.RevokeTokenResponse, error) {
	log.Printf("[INFO] Revoking Token for user: %s", req.UserID)
	JM := Manager.GetJwtManager()
	DB := Manager.GetDB()
	PrometheusCounters.TokenRevokeTotal.Inc()

	// Verify the token first
	claims, err := JM.VerifyToken(req.Token)
	if err != nil {
		PrometheusCounters.TokenRevokeFailures.Inc()
		return nil, err
	}

	if claims.UserID != req.UserID {
		PrometheusCounters.TokenRevokeFailures.Inc()
		log.Printf("[WARN] Token revocation failed for user %s: user ID mismatch", req.UserID)
		return nil, errors.New("user ID does not match token")
	}

	err = DB.RevokeToken(claims.ID)
	if err != nil {
		return nil, err
	}

	PrometheusCounters.TokenRevokeSuccess.Inc()
	log.Printf("[INFO] Token revoked successfully for user %s", req.UserID)
	return &pb.RevokeTokenResponse{}, nil
}

func (s *VMAuthServer) ActivateToken(ctx context.Context, req *pb.RevokeTokenRequest) (*pb.RevokeTokenResponse, error) {
	log.Printf("[INFO] Activating Token for user: %s", req.UserID)
	JM := Manager.GetJwtManager()
	DB := Manager.GetDB()
	PrometheusCounters.TokenRevokeTotal.Inc()

	// Verify the token first
	claims, err := JM.VerifyToken(req.Token)
	if err != nil {
		PrometheusCounters.TokenRevokeFailures.Inc()
		return nil, err
	}

	if claims.UserID != req.UserID {
		PrometheusCounters.TokenRevokeFailures.Inc()
		log.Printf("[WARN] Token activation failed for user %s: user ID mismatch", req.UserID)
		return nil, errors.New("user ID does not match token")
	}

	err = DB.ActivateToken(claims.ID)
	if err != nil {
		return nil, err
	}

	PrometheusCounters.TokenRevokeSuccess.Inc()
	log.Printf("[INFO] Token activated successfully for user %s", req.UserID)
	return &pb.RevokeTokenResponse{}, nil
}

func (s *VMAuthServer) NewSessionToken(ctx context.Context, req *pb.NewSessionTokenRequest) (*pb.NewSessionTokenResponse, error) {
	log.Printf("[INFO] Creating new Session token for : %s", req.RefreshToken)
	JM := Manager.GetJwtManager()
	DB := Manager.GetDB()

	claims, err := JM.VerifyToken(req.RefreshToken)
	if err != nil {
		PrometheusCounters.NewSessionTokenFailures.Inc()
		return nil, err
	}

	if claims.TokenType != "refresh" {
		PrometheusCounters.NewSessionTokenFailures.Inc()
		return nil, errors.New("provided token is not a refresh token")
	}

	// Check if the token is revoked
	tokenRecord, err := DB.LoadToken(claims.ID)
	if err != nil {
		PrometheusCounters.NewSessionTokenFailures.Inc()
		return nil, err
	}
	if tokenRecord.Revoked {
		PrometheusCounters.NewSessionTokenFailures.Inc()
		return nil, errors.New("refresh token is revoked")
	}
	if tokenRecord.ExpiresAt.Before(time.Now()) {
		PrometheusCounters.NewSessionTokenFailures.Inc()
		return nil, errors.New("refresh token is expired")
	}

	// Create a new session token
	PrometheusCounters.NewSessionTokenTotal.Inc()

	newSessionToken, err := JM.CreateSessionToken(req.RefreshToken)
	if err != nil {
		PrometheusCounters.NewSessionTokenSuccess.Inc()
		return nil, err
	}

	PrometheusCounters.NewSessionTokenFailures.Inc()
	log.Printf("[INFO] New session token created successfully for token %s", req.RefreshToken)
	return &pb.NewSessionTokenResponse{SessionToken: newSessionToken}, nil
}
