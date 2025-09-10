package PrometheusCounters

import "github.com/prometheus/client_golang/prometheus"

var (
	// VerifyToken
	VerifyTokenTotal = prometheus.NewCounter(prometheus.CounterOpts{
		Name: "VerifyTokenTotal",
		Help: "Total Token verification requests the API received from Clients",
	})
	VerifyTokenSuccess = prometheus.NewCounter(prometheus.CounterOpts{
		Name: "VerifyTokenSuccess",
		Help: "Successful Token verification requests the API processed",
	})
	VerifyTokenFailures = prometheus.NewCounter(prometheus.CounterOpts{
		Name: "VerifyTokenFailures",
		Help: "Failed Token verification requests the API processed",
	})

	// RefreshToken
	NewSessionTokenTotal = prometheus.NewCounter(prometheus.CounterOpts{
		Name: "NewSessionTokenTotal",
		Help: "Total NewSessionToken requests the API received from Clients",
	})
	NewSessionTokenSuccess = prometheus.NewCounter(prometheus.CounterOpts{
		Name: "NewSessionTokenSuccess",
		Help: "Successful NewSessionToken requests the API processed",
	})
	NewSessionTokenFailures = prometheus.NewCounter(prometheus.CounterOpts{
		Name: "NewSessionTokenFailures",
		Help: "Failed NewSessionToken requests the API processed",
	})

	// RevokeToken
	TokenRevokeTotal = prometheus.NewCounter(prometheus.CounterOpts{
		Name: "TokenRevokeTotal",
		Help: "Total Token revocation requests the API received from Clients",
	})
	TokenRevokeSuccess = prometheus.NewCounter(prometheus.CounterOpts{
		Name: "TokenRevokeSuccess",
		Help: "Successful Token revocation requests the API processed",
	})
	TokenRevokeFailures = prometheus.NewCounter(prometheus.CounterOpts{
		Name: "TokenRevokeFailures",
		Help: "Failed Token revocation requests the API processed",
	})

	// ActivateToken
	TokenActivateTotal = prometheus.NewCounter(prometheus.CounterOpts{
		Name: "TokenActivateTotal",
		Help: "Total Token activation requests the API received from Clients",
	})
	TokenActivateSuccess = prometheus.NewCounter(prometheus.CounterOpts{
		Name: "TokenActivateSuccess",
		Help: "Successful Token activation requests the API processed",
	})
	TokenActivateFailures = prometheus.NewCounter(prometheus.CounterOpts{
		Name: "TokenActivateFailures",
		Help: "Failed Token activation requests the API processed",
	})
)
