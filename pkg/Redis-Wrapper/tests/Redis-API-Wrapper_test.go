package RedisWrapper

import (
	RedisWrapper "RedisWrapper/src"
	"context"
	"testing"

	"github.com/alicebob/miniredis/v2"
	"github.com/stretchr/testify/assert"
)

func newTestClient(t *testing.T) *RedisWrapper.Client {
	mr, err := miniredis.Run()
	if err != nil {
		t.Fatalf("failed to start miniredis: %v", err)
	}

	t.Cleanup(mr.Close)

	client, err := RedisWrapper.New(mr.Addr(), true)
	if err != nil {
		t.Fatalf("failed to create Redis client: %v", err)
	}

	return client
}

func cleanUp(t *testing.T, client *RedisWrapper.Client, userID string) {
	err := client.RedisClient.Del(context.Background(), "user:status:"+userID).Err()
	if err != nil {
		t.Fatalf("failed to delete user status: %v", err)
	}
	err = client.RedisClient.Del(context.Background(), "user:session:"+userID).Err()
	if err != nil {
		t.Fatalf("failed to delete user session: %v", err)
	}
}

// Tests für User Status
func TestSetUserStatus_Success(t *testing.T) {
	userID := "user123"
	client := newTestClient(t)
	defer cleanUp(t, client, userID)

	err := client.SetUserStatus(userID, "online")
	assert.NoError(t, err)

	status, err := client.GetUserStatus(userID)
	assert.NoError(t, err)
	assert.Equal(t, "online", status)
}

func TestSetUserStatus_Error(t *testing.T) {
	userID := "user123"
	client := newTestClient(t)
	defer cleanUp(t, client, userID)

	err := client.SetUserStatus(userID, "")
	if err != nil {
		assert.Error(t, err)
	}
}

func TestGetUserStatus_Success(t *testing.T) {
	userID := "user123"
	client := newTestClient(t)
	defer cleanUp(t, client, userID)

	err := client.SetUserStatus(userID, "online")
	assert.NoError(t, err)

	status, err := client.GetUserStatus(userID)
	assert.NoError(t, err)
	assert.Equal(t, "online", status)
}

func TestGetUserStatus_NotFound(t *testing.T) {
	client := newTestClient(t)
	userID := "nonexistent_user"
	defer cleanUp(t, client, userID)

	status, err := client.GetUserStatus(userID)
	assert.Error(t, err)
	assert.Empty(t, status)
	assert.Contains(t, err.Error(), "failed to get user status")
}

// Tests für User Session
func TestSetUserSession_Success(t *testing.T) {
	userID := "user123"
	apiID := "api456"
	client := newTestClient(t)
	defer cleanUp(t, client, userID)

	err := client.SetUserSession(userID, apiID)
	assert.NoError(t, err)

	session, err := client.GetUserSession(userID)
	assert.NoError(t, err)
	assert.Equal(t, apiID, session)
}

func TestSetUserSession_Error(t *testing.T) {
	client := newTestClient(t)
	userID := "user123"
	apiID := ""
	defer cleanUp(t, client, userID)

	err := client.SetUserSession(userID, apiID)

	if err != nil {
		assert.Error(t, err)
	}
}

func TestGetUserSession_Success(t *testing.T) {
	userID := "user123"
	apiID := "api456"
	client := newTestClient(t)
	defer cleanUp(t, client, userID)

	err := client.SetUserSession(userID, apiID)
	assert.NoError(t, err)

	session, err := client.GetUserSession(userID)
	assert.NoError(t, err)
	assert.Equal(t, apiID, session)
}

func TestGetUserSession_NotFound(t *testing.T) {
	client := newTestClient(t)
	userID := "nonexistent_user"
	defer cleanUp(t, client, userID)

	session, err := client.GetUserSession(userID)
	assert.Error(t, err)
	assert.Empty(t, session)
	assert.Contains(t, err.Error(), "failed to get user session")
}

func TestRedisConnection(t *testing.T) {
	client := newTestClient(t)
	ctx := context.Background()
	pong, err := client.RedisClient.Ping(ctx).Result()
	assert.NoError(t, err)
	assert.Equal(t, "PONG", pong)
}
