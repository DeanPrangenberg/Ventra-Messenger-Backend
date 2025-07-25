package requests

import (
	gRPC "Redis-API-Wrapper/Redis-API-gRPC"
	"VM-REDIS-API/src/requests"
	"context"
	"github.com/alicebob/miniredis/v2"
	"github.com/redis/go-redis/v9"
	"testing"
)

func setupTestServer(t *testing.T) *requests.Server {
	s, err := miniredis.Run()
	if err != nil {
		t.Fatalf("failed to start miniredis: %v", err)
	}

	rdb := redis.NewClient(&redis.Options{
		Addr: s.Addr(),
	})
	return &requests.Server{RedisClient: rdb}
}

func TestSetAndGetUserStatus(t *testing.T) {
	s := setupTestServer(t)
	ctx := context.Background()

	_, err := s.SetUserStatus(ctx, &gRPC.UserStatusRequest{
		UserId: "user1",
		Status: "online",
	})
	if err != nil {
		t.Fatal(err)
	}

	resp, err := s.GetUserStatus(ctx, &gRPC.UserID{UserId: "user1"})
	if err != nil {
		t.Fatal(err)
	}
	if resp.Status != "online" {
		t.Errorf("expected status 'online', got %s", resp.Status)
	}
}

func TestSetAndGetUserSession(t *testing.T) {
	s := setupTestServer(t)
	ctx := context.Background()

	_, err := s.SetUserSession(ctx, &gRPC.SetUserSessionRequest{
		UserId: "user42",
		Api:    "api-abc",
	})
	if err != nil {
		t.Fatal(err)
	}

	resp, err := s.GetUserSession(ctx, &gRPC.GetUserSessionRequest{
		UserId: "user42",
	})
	if err != nil {
		t.Fatal(err)
	}
	if resp.Api != "api-abc" {
		t.Errorf("expected API 'api-abc', got %s", resp.Api)
	}
}

func TestMetricOperations(t *testing.T) {
	s := setupTestServer(t)
	ctx := context.Background()

	// Set metric
	_, err := s.UpdateMetric(ctx, &gRPC.MetricUpdateRequest{
		Key:   "cpu",
		Value: 100,
	})
	if err != nil {
		t.Fatal(err)
	}

	// Increment metric
	_, err = s.IncrementMetric(ctx, &gRPC.MetricUpdateRequest{
		Key:   "cpu",
		Value: 10,
	})
	if err != nil {
		t.Fatal(err)
	}

	// Get metric
	metricsResp, err := s.GetMetrics(ctx, &gRPC.GetMetricsRequest{
		Keys: []string{"cpu"},
	})
	if err != nil {
		t.Fatal(err)
	}

	val := metricsResp.Metrics["cpu"]
	if val != 110 {
		t.Errorf("expected cpu = 110, got %d", val)
	}
}

func TestResetMetric(t *testing.T) {
	s := setupTestServer(t)
	ctx := context.Background()

	_, err := s.UpdateMetric(ctx, &gRPC.MetricUpdateRequest{
		Key:   "ram",
		Value: 512,
	})
	if err != nil {
		t.Fatal(err)
	}

	_, err = s.ResetMetric(ctx, &gRPC.MetricKeyRequest{
		Key: "ram",
	})
	if err != nil {
		t.Fatal(err)
	}

	metricsResp, err := s.GetMetrics(ctx, &gRPC.GetMetricsRequest{
		Keys: []string{"ram"},
	})
	if err != nil {
		t.Fatal(err)
	}

	if metricsResp.Metrics["ram"] != 0 {
		t.Errorf("expected ram = 0, got %d", metricsResp.Metrics["ram"])
	}
}

func TestGetAllMetrics(t *testing.T) {
	s := setupTestServer(t)
	ctx := context.Background()

	// multiple keys
	s.UpdateMetric(ctx, &gRPC.MetricUpdateRequest{Key: "net", Value: 200})
	s.UpdateMetric(ctx, &gRPC.MetricUpdateRequest{Key: "disk", Value: 300})

	resp, err := s.GetAllMetrics(ctx, &gRPC.Empty{})
	if err != nil {
		t.Fatal(err)
	}

	if resp.Metrics["net"] != 200 || resp.Metrics["disk"] != 300 {
		t.Errorf("unexpected metrics: %+v", resp.Metrics)
	}
}
