package RedisWrapper

import (
	gRPC "Redis-API-Wrapper/Redis-API-gRPC"
	"fmt"
)

func (c *Client) IncrementMetric(metricName string, value int64) error {
	req := &gRPC.MetricUpdateRequest{
		Key:   metricName,
		Value: value,
	}

	resp, err := c.Client.IncrementMetric(c.Ctx, req)
	if err != nil {
		return fmt.Errorf("failed to increment metric %s: %w", metricName, err)
	}

	if resp == nil {
		return fmt.Errorf("received nil response when incrementing metric %s", metricName)
	}

	if c.DebugPrints {
		fmt.Println(resp)
	}

	return nil
}

func (c *Client) UpdateMetric(metricName string, value int64) error {
	req := &gRPC.MetricUpdateRequest{
		Key:   metricName,
		Value: value,
	}

	resp, err := c.Client.UpdateMetric(c.Ctx, req)
	if err != nil {
		return fmt.Errorf("failed to update metric %s: %w", metricName, err)
	}

	if resp == nil {
		return fmt.Errorf("received nil response when updating metric %s", metricName)
	}

	if c.DebugPrints {
		fmt.Println(resp)
	}

	return nil
}

func (c *Client) ResetMetric(metricName string) error {
	req := &gRPC.MetricKeyRequest{
		Key: metricName,
	}

	resp, err := c.Client.ResetMetric(c.Ctx, req)
	if err != nil {
		return fmt.Errorf("failed to reset metric %s: %w", metricName, err)
	}

	if resp == nil {
		return fmt.Errorf("received nil response when resetting metric %s", metricName)
	}

	if c.DebugPrints {
		fmt.Println(resp)
	}

	return nil
}

func (c *Client) GetMetrics(metrics []string) (map[string]int64, error) {
	req := &gRPC.GetMetricsRequest{
		Keys: metrics,
	}

	resp, err := c.Client.GetMetrics(c.Ctx, req)
	if err != nil {
		return nil, fmt.Errorf("failed to get metrics: %w", err)
	}

	if resp == nil {
		return nil, fmt.Errorf("received nil response when getting metrics")
	}

	if c.DebugPrints {
		fmt.Println(resp.Metrics)
	}

	// Sicherstellen, dass Metrics nicht nil ist
	if resp.Metrics == nil {
		return make(map[string]int64), nil
	}

	return resp.Metrics, nil
}

func (c *Client) GetAllMetrics() (map[string]int64, error) {
	req := &gRPC.Empty{}

	resp, err := c.Client.GetAllMetrics(c.Ctx, req)
	if err != nil {
		return nil, fmt.Errorf("failed to get all metrics: %w", err)
	}

	if resp == nil {
		return nil, fmt.Errorf("received nil response when getting all metrics")
	}

	if c.DebugPrints {
		fmt.Println(resp.Metrics)
	}

	// Sicherstellen, dass Metrics nicht nil ist
	if resp.Metrics == nil {
		return make(map[string]int64), nil
	}

	return resp.Metrics, nil
}
