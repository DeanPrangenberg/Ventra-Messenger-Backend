package RedisWrapper

import (
	RedisWrapper "Redis-API-Wrapper/src"
	"context"
	"errors"
	"testing"

	Redis_gRPC "Redis-API-Wrapper/Redis-API-gRPC"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
	"google.golang.org/grpc"
)

// Mock für den gRPC-Client
type mockUserStatusServiceClient struct {
	mock.Mock
}

func (m *mockUserStatusServiceClient) SetUserStatus(ctx context.Context, in *Redis_gRPC.UserStatusRequest, opts ...grpc.CallOption) (*Redis_gRPC.StatusResponse, error) {
	args := m.Called(ctx, in)
	if args.Get(0) == nil {
		return nil, args.Error(1)
	}
	return args.Get(0).(*Redis_gRPC.StatusResponse), args.Error(1)
}

func (m *mockUserStatusServiceClient) GetUserStatus(ctx context.Context, in *Redis_gRPC.UserID, opts ...grpc.CallOption) (*Redis_gRPC.UserStatusResponse, error) {
	args := m.Called(ctx, in)
	if args.Get(0) == nil {
		return nil, args.Error(1)
	}
	return args.Get(0).(*Redis_gRPC.UserStatusResponse), args.Error(1)
}

func (m *mockUserStatusServiceClient) SetUserSession(ctx context.Context, in *Redis_gRPC.SetUserSessionRequest, opts ...grpc.CallOption) (*Redis_gRPC.StatusResponse, error) {
	args := m.Called(ctx, in)
	if args.Get(0) == nil {
		return nil, args.Error(1)
	}
	return args.Get(0).(*Redis_gRPC.StatusResponse), args.Error(1)
}

func (m *mockUserStatusServiceClient) GetUserSession(ctx context.Context, in *Redis_gRPC.GetUserSessionRequest, opts ...grpc.CallOption) (*Redis_gRPC.GetUserSessionResponse, error) {
	args := m.Called(ctx, in)
	if args.Get(0) == nil {
		return nil, args.Error(1)
	}
	return args.Get(0).(*Redis_gRPC.GetUserSessionResponse), args.Error(1)
}

func (m *mockUserStatusServiceClient) IncrementMetric(ctx context.Context, in *Redis_gRPC.MetricUpdateRequest, opts ...grpc.CallOption) (*Redis_gRPC.StatusResponse, error) {
	args := m.Called(ctx, in)
	if args.Get(0) == nil {
		return nil, args.Error(1)
	}
	return args.Get(0).(*Redis_gRPC.StatusResponse), args.Error(1)
}

func (m *mockUserStatusServiceClient) UpdateMetric(ctx context.Context, in *Redis_gRPC.MetricUpdateRequest, opts ...grpc.CallOption) (*Redis_gRPC.StatusResponse, error) {
	args := m.Called(ctx, in)
	if args.Get(0) == nil {
		return nil, args.Error(1)
	}
	return args.Get(0).(*Redis_gRPC.StatusResponse), args.Error(1)
}

func (m *mockUserStatusServiceClient) ResetMetric(ctx context.Context, in *Redis_gRPC.MetricKeyRequest, opts ...grpc.CallOption) (*Redis_gRPC.StatusResponse, error) {
	args := m.Called(ctx, in)
	if args.Get(0) == nil {
		return nil, args.Error(1)
	}
	return args.Get(0).(*Redis_gRPC.StatusResponse), args.Error(1)
}

func (m *mockUserStatusServiceClient) GetMetrics(ctx context.Context, in *Redis_gRPC.GetMetricsRequest, opts ...grpc.CallOption) (*Redis_gRPC.MetricsResponse, error) {
	args := m.Called(ctx, in)
	if args.Get(0) == nil {
		return nil, args.Error(1)
	}
	return args.Get(0).(*Redis_gRPC.MetricsResponse), args.Error(1)
}

func (m *mockUserStatusServiceClient) GetAllMetrics(ctx context.Context, in *Redis_gRPC.Empty, opts ...grpc.CallOption) (*Redis_gRPC.MetricsResponse, error) {
	args := m.Called(ctx, in)
	if args.Get(0) == nil {
		return nil, args.Error(1)
	}
	return args.Get(0).(*Redis_gRPC.MetricsResponse), args.Error(1)
}

// Test-Helper Funktion
func newTestClient() (*mockUserStatusServiceClient, *RedisWrapper.Client) {
	mockClient := &mockUserStatusServiceClient{}
	client := &RedisWrapper.Client{
		Client:      mockClient,
		Ctx:         context.Background(),
		DebugPrints: false,
	}
	return mockClient, client
}

// Tests für User Status
func TestSetUserStatus_Success(t *testing.T) {
	mockClient, client := newTestClient()

	req := &Redis_gRPC.UserStatusRequest{
		UserId: "user123",
		Status: "online",
	}
	resp := &Redis_gRPC.StatusResponse{
		Success: true,
		Message: "Status updated",
	}

	mockClient.On("SetUserStatus", mock.Anything, req).Return(resp, nil)

	err := client.SetUserStatus("user123", "online")
	assert.NoError(t, err)
	mockClient.AssertExpectations(t)
}

func TestSetUserStatus_NetworkError(t *testing.T) {
	mockClient, client := newTestClient()

	req := &Redis_gRPC.UserStatusRequest{
		UserId: "user123",
		Status: "online",
	}

	mockClient.On("SetUserStatus", mock.Anything, req).Return((*Redis_gRPC.StatusResponse)(nil), errors.New("connection failed"))

	err := client.SetUserStatus("user123", "online")
	assert.Error(t, err)
	assert.Contains(t, err.Error(), "failed to set status session")
	mockClient.AssertExpectations(t)
}

func TestSetUserStatus_BusinessError(t *testing.T) {
	mockClient, client := newTestClient()

	req := &Redis_gRPC.UserStatusRequest{
		UserId: "user123",
		Status: "online",
	}
	resp := &Redis_gRPC.StatusResponse{
		Success: false,
		Message: "Invalid status",
	}

	mockClient.On("SetUserStatus", mock.Anything, req).Return(resp, nil)

	err := client.SetUserStatus("user123", "online")
	assert.Error(t, err)
	assert.Contains(t, err.Error(), "Invalid status")
	mockClient.AssertExpectations(t)
}

func TestGetUserStatus_Success(t *testing.T) {
	mockClient, client := newTestClient()

	req := &Redis_gRPC.UserID{
		UserId: "user123",
	}
	resp := &Redis_gRPC.UserStatusResponse{
		Status: "online",
	}

	mockClient.On("GetUserStatus", mock.Anything, req).Return(resp, nil)

	status, err := client.GetUserStatus("user123")
	assert.NoError(t, err)
	assert.Equal(t, "online", status)
	mockClient.AssertExpectations(t)
}

func TestGetUserStatus_Error(t *testing.T) {
	mockClient, client := newTestClient()

	req := &Redis_gRPC.UserID{
		UserId: "user123",
	}

	mockClient.On("GetUserStatus", mock.Anything, req).Return((*Redis_gRPC.UserStatusResponse)(nil), errors.New("not found"))

	_, err := client.GetUserStatus("user123")
	assert.Error(t, err)
	assert.Contains(t, err.Error(), "failed to get api session")
	mockClient.AssertExpectations(t)
}

// Tests für User Session
func TestSetUserSession_Success(t *testing.T) {
	mockClient, client := newTestClient()

	req := &Redis_gRPC.SetUserSessionRequest{
		UserId: "user123",
		Api:    "api456",
	}
	resp := &Redis_gRPC.StatusResponse{
		Success: true,
		Message: "Session set successfully",
	}

	mockClient.On("SetUserSession", mock.Anything, req).Return(resp, nil)

	err := client.SetUserSession("user123", "api456")
	assert.NoError(t, err)
	mockClient.AssertExpectations(t)
}

func TestSetUserSession_NetworkError(t *testing.T) {
	mockClient, client := newTestClient()

	req := &Redis_gRPC.SetUserSessionRequest{
		UserId: "user123",
		Api:    "api456",
	}

	mockClient.On("SetUserSession", mock.Anything, req).Return((*Redis_gRPC.StatusResponse)(nil), errors.New("connection failed"))

	err := client.SetUserSession("user123", "api456")
	assert.Error(t, err)
	assert.Contains(t, err.Error(), "failed to set api session")
	mockClient.AssertExpectations(t)
}

func TestSetUserSession_BusinessError(t *testing.T) {
	mockClient, client := newTestClient()

	req := &Redis_gRPC.SetUserSessionRequest{
		UserId: "user123",
		Api:    "api456",
	}
	resp := &Redis_gRPC.StatusResponse{
		Success: false,
		Message: "Invalid API key",
	}

	mockClient.On("SetUserSession", mock.Anything, req).Return(resp, nil)

	err := client.SetUserSession("user123", "api456")
	assert.Error(t, err)
	assert.Contains(t, err.Error(), "Invalid API key")
	mockClient.AssertExpectations(t)
}

func TestGetUserSession_Success(t *testing.T) {
	mockClient, client := newTestClient()

	req := &Redis_gRPC.GetUserSessionRequest{
		UserId: "user123",
	}
	resp := &Redis_gRPC.GetUserSessionResponse{
		Api: "api456",
	}

	mockClient.On("GetUserSession", mock.Anything, req).Return(resp, nil)

	api, err := client.GetUserSession("user123")
	assert.NoError(t, err)
	assert.Equal(t, "api456", api)
	mockClient.AssertExpectations(t)
}

func TestGetUserSession_Error(t *testing.T) {
	mockClient, client := newTestClient()

	req := &Redis_gRPC.GetUserSessionRequest{
		UserId: "user123",
	}

	mockClient.On("GetUserSession", mock.Anything, req).Return((*Redis_gRPC.GetUserSessionResponse)(nil), errors.New("not found"))

	_, err := client.GetUserSession("user123")
	assert.Error(t, err)
	assert.Contains(t, err.Error(), "failed to get api session")
	mockClient.AssertExpectations(t)
}

// Tests für Metrics
func TestIncrementMetric_Success(t *testing.T) {
	mockClient, client := newTestClient()

	req := &Redis_gRPC.MetricUpdateRequest{
		Key:   "counter",
		Value: 5,
	}
	resp := &Redis_gRPC.StatusResponse{
		Success: true,
		Message: "Metric incremented",
	}

	mockClient.On("IncrementMetric", mock.Anything, req).Return(resp, nil)

	err := client.IncrementMetric("counter", 5)
	assert.NoError(t, err)
	mockClient.AssertExpectations(t)
}

func TestUpdateMetric_Success(t *testing.T) {
	mockClient, client := newTestClient()

	req := &Redis_gRPC.MetricUpdateRequest{
		Key:   "gauge",
		Value: 100,
	}
	resp := &Redis_gRPC.StatusResponse{
		Success: true,
		Message: "Metric updated",
	}

	mockClient.On("UpdateMetric", mock.Anything, req).Return(resp, nil)

	err := client.UpdateMetric("gauge", 100)
	assert.NoError(t, err)
	mockClient.AssertExpectations(t)
}

func TestResetMetric_Success(t *testing.T) {
	mockClient, client := newTestClient()

	req := &Redis_gRPC.MetricKeyRequest{
		Key: "counter",
	}
	resp := &Redis_gRPC.StatusResponse{
		Success: true,
		Message: "Metric reset",
	}

	mockClient.On("ResetMetric", mock.Anything, req).Return(resp, nil)

	err := client.ResetMetric("counter")
	assert.NoError(t, err)
	mockClient.AssertExpectations(t)
}

func TestGetMetrics_Success(t *testing.T) {
	mockClient, client := newTestClient()

	req := &Redis_gRPC.GetMetricsRequest{
		Keys: []string{"metric1", "metric2"},
	}
	metrics := map[string]int64{
		"metric1": 10,
		"metric2": 20,
	}
	resp := &Redis_gRPC.MetricsResponse{
		Metrics: metrics,
	}

	mockClient.On("GetMetrics", mock.Anything, req).Return(resp, nil)

	result, err := client.GetMetrics([]string{"metric1", "metric2"})
	assert.NoError(t, err)
	assert.Equal(t, metrics, result)
	mockClient.AssertExpectations(t)
}

func TestGetMetrics_EmptyResult(t *testing.T) {
	mockClient, client := newTestClient()

	req := &Redis_gRPC.GetMetricsRequest{
		Keys: []string{"metric1"},
	}
	resp := &Redis_gRPC.MetricsResponse{
		Metrics: nil,
	}

	mockClient.On("GetMetrics", mock.Anything, req).Return(resp, nil)

	result, err := client.GetMetrics([]string{"metric1"})
	assert.NoError(t, err)
	assert.Empty(t, result)
	mockClient.AssertExpectations(t)
}

func TestGetAllMetrics_Success(t *testing.T) {
	mockClient, client := newTestClient()

	req := &Redis_gRPC.Empty{}
	metrics := map[string]int64{
		"total_users":  1000,
		"active_users": 500,
	}
	resp := &Redis_gRPC.MetricsResponse{
		Metrics: metrics,
	}

	mockClient.On("GetAllMetrics", mock.Anything, req).Return(resp, nil)

	result, err := client.GetAllMetrics()
	assert.NoError(t, err)
	assert.Equal(t, metrics, result)
	mockClient.AssertExpectations(t)
}

func TestGetAllMetrics_NetworkError(t *testing.T) {
	mockClient, client := newTestClient()

	req := &Redis_gRPC.Empty{}

	mockClient.On("GetAllMetrics", mock.Anything, req).Return((*Redis_gRPC.MetricsResponse)(nil), errors.New("service unavailable"))

	_, err := client.GetAllMetrics()
	assert.Error(t, err)
	assert.Contains(t, err.Error(), "failed to get all metrics")
	mockClient.AssertExpectations(t)
}
