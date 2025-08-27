package VM_API_gRPC_tests

import (
	"context"
	"errors"
	"testing"

	pb "VM-API-gRPC-Wrapper/gen-pb"
	VM_API_gRPC_Wrapper "VM-API-gRPC-Wrapper/src"

	"google.golang.org/grpc"
)

// Mock UserApiClient
type mockUserApiClient struct {
	isConnectedResp *pb.UserStatusResponse
	isConnectedErr  error
	sendPayloadResp *pb.PayloadResponse
	sendPayloadErr  error
}

func (m *mockUserApiClient) IsUserConnected(ctx context.Context, in *pb.UserStatusRequest, opts ...grpc.CallOption) (*pb.UserStatusResponse, error) {
	return m.isConnectedResp, m.isConnectedErr
}
func (m *mockUserApiClient) SendPayload(ctx context.Context, in *pb.PayloadRequest, opts ...grpc.CallOption) (*pb.PayloadResponse, error) {
	return m.sendPayloadResp, m.sendPayloadErr
}

func TestIsUserConnected(t *testing.T) {
	client := &VM_API_gRPC_Wrapper.VmApiClient{
		Client: &mockUserApiClient{
			isConnectedResp: &pb.UserStatusResponse{Connected: true},
			isConnectedErr:  nil,
		},
	}
	connected, err := client.IsUserConnected("user1")
	if err != nil {
		t.Fatalf("unexpected error: %v", err)
	}
	if !connected {
		t.Errorf("expected connected=true, got false")
	}
}

func TestIsUserConnected_Error(t *testing.T) {
	client := &VM_API_gRPC_Wrapper.VmApiClient{
		Client: &mockUserApiClient{
			isConnectedResp: nil,
			isConnectedErr:  errors.New("fail"),
		},
	}
	_, err := client.IsUserConnected("user1")
	if err == nil {
		t.Errorf("expected error, got nil")
	}
}

func TestSendPayload(t *testing.T) {
	client := &VM_API_gRPC_Wrapper.VmApiClient{
		Client: &mockUserApiClient{
			sendPayloadResp: &pb.PayloadResponse{Ack: "ok"},
			sendPayloadErr:  nil,
		},
	}
	ack, err := client.SendPayload("user1", []byte("data"))
	if err != nil {
		t.Fatalf("unexpected error: %v", err)
	}
	if ack != "ok" {
		t.Errorf("expected ack=ok, got %s", ack)
	}
}

func TestSendPayload_Error(t *testing.T) {
	client := &VM_API_gRPC_Wrapper.VmApiClient{
		Client: &mockUserApiClient{
			sendPayloadResp: nil,
			sendPayloadErr:  errors.New("fail"),
		},
	}
	_, err := client.SendPayload("user1", []byte("data"))
	if err == nil {
		t.Errorf("expected error, got nil")
	}
}
