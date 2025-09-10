package VM_AUTH_gRPC_tests

import (
	"context"
	"errors"
	"testing"

	pb "VM-AUTH-gRPC-Wrapper/gen-pb"
	VM_AUTH_gRPC_Wrapper "VM-AUTH-gRPC-Wrapper/src"

	"google.golang.org/grpc"
)

// Mock UserAuthClient
type mockUserAuthClient struct {
	isConnectedResp *pb.UserStatusResponse
	isConnectedErr  error
	sendPayloadResp *pb.PayloadResponse
	sendPayloadErr  error
}

func (m *mockUserAuthClient) IsUserConnected(ctx context.Context, in *pb.UserStatusRequest, opts ...grpc.CallOption) (*pb.UserStatusResponse, error) {
	return m.isConnectedResp, m.isConnectedErr
}
func (m *mockUserAuthClient) SendPayload(ctx context.Context, in *pb.PayloadRequest, opts ...grpc.CallOption) (*pb.PayloadResponse, error) {
	return m.sendPayloadResp, m.sendPayloadErr
}

func TestIsUserConnected(t *testing.T) {
	client := &VM_AUTH_gRPC_Wrapper.VmAuthClient{
		Client: &mockUserAuthClient{
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
	client := &VM_AUTH_gRPC_Wrapper.VmAuthClient{
		Client: &mockUserAuthClient{
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
	client := &VM_AUTH_gRPC_Wrapper.VmAuthClient{
		Client: &mockUserAuthClient{
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
	client := &VM_AUTH_gRPC_Wrapper.VmAuthClient{
		Client: &mockUserAuthClient{
			sendPayloadResp: nil,
			sendPayloadErr:  errors.New("fail"),
		},
	}
	_, err := client.SendPayload("user1", []byte("data"))
	if err == nil {
		t.Errorf("expected error, got nil")
	}
}
