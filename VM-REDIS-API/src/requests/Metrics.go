package requests

import (
	gRPC "Redis-API-Wrapper/Redis-API-gRPC"
	"context"
	"fmt"
	"strconv"
)

func (s *Server) IncrementMetric(ctx context.Context, req *gRPC.MetricUpdateRequest) (*gRPC.StatusResponse, error) {
	err := s.RedisClient.HIncrBy(ctx, "metrics", req.Key, req.Value).Err()
	if err != nil {
		return nil, err
	}

	return &gRPC.StatusResponse{
		Message: fmt.Sprint("Increment Metric", req.Key, "by", req.Value),
		Success: true,
	}, nil
}

func (s *Server) UpdateMetric(ctx context.Context, req *gRPC.MetricUpdateRequest) (*gRPC.StatusResponse, error) {
	err := s.RedisClient.HSet(ctx, "metrics", req.Key, req.Value).Err()
	if err != nil {
		return nil, err
	}

	return &gRPC.StatusResponse{
		Message: fmt.Sprint("Update Metric", req.Key, "to", req.Value),
		Success: true,
	}, nil
}

func (s *Server) ResetMetric(ctx context.Context, req *gRPC.MetricKeyRequest) (*gRPC.StatusResponse, error) {
	err := s.RedisClient.HSet(ctx, "metrics", req.Key, 0).Err()
	if err != nil {
		return nil, err
	}

	return &gRPC.StatusResponse{
		Message: fmt.Sprint("Rested Metric", req.Key),
		Success: true,
	}, nil
}

func (s *Server) GetMetrics(ctx context.Context, req *gRPC.GetMetricsRequest) (*gRPC.MetricsResponse, error) {
	vals, err := s.RedisClient.HMGet(ctx, "metrics", req.Keys...).Result()
	if err != nil {
		return nil, err
	}

	metrics := make(map[string]int64, len(req.Keys))
	for i, v := range vals {
		if v == nil {
			continue
		}
		strVal, ok := v.(string)
		if !ok {
			continue
		}
		num, convErr := strconv.ParseInt(strVal, 10, 64)
		if convErr != nil {
			continue
		}
		metrics[req.Keys[i]] = num
	}

	return &gRPC.MetricsResponse{
		Metrics: metrics,
	}, nil
}

func (s *Server) GetAllMetrics(ctx context.Context, req *gRPC.Empty) (*gRPC.MetricsResponse, error) {
	vals, err := s.RedisClient.HGetAll(ctx, "metrics").Result()
	if err != nil {
		return nil, err
	}

	metrics := make(map[string]int64, len(vals))
	for key, strVal := range vals {
		num, convErr := strconv.ParseInt(strVal, 10, 64)
		if convErr != nil {
			continue
		}
		metrics[key] = num
	}

	return &gRPC.MetricsResponse{
		Metrics: metrics,
	}, nil
}
