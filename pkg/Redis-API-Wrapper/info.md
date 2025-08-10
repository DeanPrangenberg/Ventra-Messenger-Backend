# Redis-API-Wrapper

### Info

1. This wrapper provides a simple and secure interface to interact with the Redis gRPC API.
2. It includes a pre-built Go client library with easy-to-use methods for all available RPCs.
3. The wrapper handles error checking, nil-pointer safety, and proper context management.
4. Each function comes with comprehensive unit tests located in the `test` folder to verify functionality.
5. The wrapper can be easily extended with additional methods as new gRPC services are added.
6. Built-in debug mode for development and troubleshooting.

### Usage
```go
package main

import (
    "context"
    "fmt"
    "log"
    "time"
    
    "Redis-API-Wrapper/src/RedisWrapper"
)

func main() {
    // Create a new client
    client, err := RedisWrapper.New("localhost:50051", false)
    if err != nil {
        log.Fatalf("Failed to create client: %v", err)
    }
    defer client.Close()

    // Set user status
    err = client.SetUserStatus("user123", "online")
    if err != nil {
        log.Printf("Error setting user status: %v", err)
    } else {
        fmt.Println("User status set to online")
    }

    // Get user status
    status, err := client.GetUserStatus("user123")
    if err != nil {
        log.Printf("Error getting user status: %v", err)
    } else {
        fmt.Printf("User status: %s\n", status)
    }

    // Set user session
    err = client.SetUserSession("user123", "api-key-abc123")
    if err != nil {
        log.Printf("Error setting user session: %v", err)
    } else {
        fmt.Println("User session set successfully")
    }

    // Work with metrics
    err = client.IncrementMetric("page_views", 1)
    if err != nil {
        log.Printf("Error incrementing metric: %v", err)
    }

    err = client.UpdateMetric("user_count", 1500)
    if err != nil {
        log.Printf("Error updating metric: %v", err)
    }

    // Get specific metrics
    metrics, err := client.GetMetrics([]string{"page_views", "user_count"})
    if err != nil {
        log.Printf("Error getting metrics: %v", err)
    } else {
        fmt.Printf("Metrics: %+v\n", metrics)
    }

    // Get all metrics
    allMetrics, err := client.GetAllMetrics()
    if err != nil {
        log.Printf("Error getting all metrics: %v", err)
    } else {
        fmt.Printf("All metrics: %+v\n", allMetrics)
    }
}
```