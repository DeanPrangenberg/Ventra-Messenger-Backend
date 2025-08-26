package PrometheusEndpoint

import "github.com/prometheus/client_golang/prometheus"

var (
	// Gauge metrics
	// Example usage:
	// PrometheusEndpoint.ConnectedClients.Set(12) // to set the number of connected clients
	// PrometheusEndpoint.ConnectedClients.Inc() // to increment the number of connected clients
	// PrometheusEndpoint.ConnectedClients.Dec() // to decrement the number of connected clients
	// PrometheusEndpoint.ConnectedClients.Add() // to add a specific number of connected clients
	// PrometheusEndpoint.ConnectedClients.Sub() // to subtract a specific number of connected clients

	ConnectedClients = prometheus.NewGauge(prometheus.GaugeOpts{
		Name: "ConnectedClients",
		Help: "Counts the API Connected Clients",
	})

	// Counter metrics
	// Example usage:
	// PrometheusEndpoint.PayloadsReceived.Inc() // to increment the count of received payload

	PayloadsReceived = prometheus.NewCounter(prometheus.CounterOpts{
		Name: "PayloadsReceived",
		Help: "Counts how many Payloads the API received from Clients",
	})

	PayloadsProcessedSuccessfully = prometheus.NewCounter(prometheus.CounterOpts{
		Name: "PayloadsProcessedSuccessfully",
		Help: "Counts how many Payloads the API received from Clients",
	})

	PayloadsProcessedFailed = prometheus.NewCounter(prometheus.CounterOpts{
		Name: "PayloadsProcessedFailed",
		Help: "Counts how many Payloads the API received from Clients",
	})

	PayloadsSendToClient = prometheus.NewCounter(prometheus.CounterOpts{
		Name: "PayloadsSendToClient",
		Help: "Counts to the API Connected Clients",
	})

	allCounters = []prometheus.Counter{
		PayloadsReceived,
		PayloadsProcessedSuccessfully,
		PayloadsProcessedFailed,
		PayloadsSendToClient,
	}

	allGauges = []prometheus.Gauge{
		ConnectedClients,
	}
)
