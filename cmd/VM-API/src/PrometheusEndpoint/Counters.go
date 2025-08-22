package PrometheusEndpoint

import "github.com/prometheus/client_golang/prometheus"

var (
	ConnectedClient = prometheus.NewCounter(prometheus.CounterOpts{
		Name: "ConnectedClient",
		Help: "Counts the API Connected Clients",
	})

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
		ConnectedClient,
		PayloadsReceived,
		PayloadsProcessedSuccessfully,
		PayloadsProcessedFailed,
		PayloadsSendToClient,
	}
)
