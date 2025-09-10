package PrometheusCounters

import "github.com/prometheus/client_golang/prometheus"

var (
	// Logins
	LoginAttemptsTotal = prometheus.NewCounter(prometheus.CounterOpts{
		Name: "LoginAttemptsTotal",
		Help: "Total Login attempts the API received from Clients",
	})
	LoginSuccess = prometheus.NewCounter(prometheus.CounterOpts{
		Name: "LoginSuccess",
		Help: "Successful Logins the API processed",
	})
	LoginFailures = prometheus.NewCounter(prometheus.CounterOpts{
		Name: "LoginFailures",
		Help: "Failed Logins the API processed",
	})

	// Register
	RegisterAttemptsTotal = prometheus.NewCounter(prometheus.CounterOpts{
		Name: "RegisterAttemptsTotal",
		Help: "Total Register attempts the API received from Clients",
	})
	RegisterSuccess = prometheus.NewCounter(prometheus.CounterOpts{
		Name: "RegisterSuccess",
		Help: "Successful Register the API processed",
	})
	RegisterFailures = prometheus.NewCounter(prometheus.CounterOpts{
		Name: "RegisterFailures",
		Help: "Failed Register the API processed",
	})

	// Update Password
	UpdatePasswordAttemptsTotal = prometheus.NewCounter(prometheus.CounterOpts{
		Name: "UpdatePasswordAttemptsTotal",
		Help: "Total Update Password attempts the API received from Clients",
	})
	UpdatePasswordSuccess = prometheus.NewCounter(prometheus.CounterOpts{
		Name: "UpdatePasswordSuccess",
		Help: "Successful Update Password the API processed",
	})
	UpdatePasswordFailures = prometheus.NewCounter(prometheus.CounterOpts{
		Name: "UpdatePasswordFailures",
		Help: "Failed Update Password the API processed",
	})

	// Update Username
	UpdateUsernameAttemptsTotal = prometheus.NewCounter(prometheus.CounterOpts{
		Name: "UpdateUsernameAttemptsTotal",
		Help: "Total Update Username attempts the API received from Clients",
	})
	UpdateUsernameSuccess = prometheus.NewCounter(prometheus.CounterOpts{
		Name: "UpdateUsernameSuccess",
		Help: "Successful Update Username the API processed",
	})
	UpdateUsernameFailures = prometheus.NewCounter(prometheus.CounterOpts{
		Name: "UpdateUsernameFailures",
		Help: "Failed Update Username the API processed",
	})

	// Delete Account
	DeleteAccountAttemptsTotal = prometheus.NewCounter(prometheus.CounterOpts{
		Name: "DeleteAccountAttemptsTotal",
		Help: "Total Delete Account attempts the API received from Clients",
	})
	DeleteAccountSuccess = prometheus.NewCounter(prometheus.CounterOpts{
		Name: "DeleteAccountSuccess",
		Help: "Successful Delete Account the API processed",
	})
	DeleteAccountFailures = prometheus.NewCounter(prometheus.CounterOpts{
		Name: "DeleteAccountFailures",
		Help: "Failed Delete Account the API processed",
	})
)
