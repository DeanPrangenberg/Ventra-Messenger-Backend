package PrometheusEndpoint

import (
	"VM-AUTH/src/PrometheusEndpoint/PrometheusCounters"
	"fmt"
	"net/http"

	"github.com/prometheus/client_golang/prometheus"
	"github.com/prometheus/client_golang/prometheus/promhttp"
)

func initMetrics() {
	allCounters := []prometheus.Counter{
		// Account PrometheusCounters
		PrometheusCounters.LoginAttemptsTotal,
		PrometheusCounters.LoginSuccess,
		PrometheusCounters.LoginFailures,
		PrometheusCounters.RegisterAttemptsTotal,
		PrometheusCounters.RegisterSuccess,
		PrometheusCounters.RegisterFailures,
		PrometheusCounters.UpdatePasswordAttemptsTotal,
		PrometheusCounters.UpdatePasswordSuccess,
		PrometheusCounters.UpdatePasswordFailures,
		PrometheusCounters.UpdateUsernameAttemptsTotal,
		PrometheusCounters.UpdateUsernameSuccess,
		PrometheusCounters.UpdateUsernameFailures,
		PrometheusCounters.DeleteAccountAttemptsTotal,
		PrometheusCounters.DeleteAccountSuccess,
		PrometheusCounters.DeleteAccountFailures,
		PrometheusCounters.AccountRevokeTotal,
		PrometheusCounters.AccountRevokeSuccess,
		PrometheusCounters.AccountRevokeFailures,
		PrometheusCounters.AccountActivateTotal,
		PrometheusCounters.AccountActivateSuccess,
		PrometheusCounters.AccountActivateFailures,

		// Token PrometheusCounters
		PrometheusCounters.VerifyTokenTotal,
		PrometheusCounters.VerifyTokenSuccess,
		PrometheusCounters.VerifyTokenFailures,
		PrometheusCounters.NewSessionTokenTotal,
		PrometheusCounters.NewSessionTokenSuccess,
		PrometheusCounters.NewSessionTokenFailures,
		PrometheusCounters.TokenRevokeTotal,
		PrometheusCounters.TokenRevokeSuccess,
		PrometheusCounters.TokenRevokeFailures,
		PrometheusCounters.TokenActivateTotal,
		PrometheusCounters.TokenActivateSuccess,
		PrometheusCounters.TokenActivateFailures,
	}

	allGauges := []prometheus.Gauge{}

	for _, counter := range allCounters {
		if err := prometheus.Register(counter); err != nil {
			fmt.Printf("Error registering counter %s: %v\n", counter.Desc().String(), err)
		}
	}

	for _, gauge := range allGauges {
		if err := prometheus.Register(gauge); err != nil {
			fmt.Printf("Error registering gauge %s: %v\n", gauge.Desc().String(), err)
		}
	}
}

func StartPrometheusEndpoint() {
	initMetrics()
	http.Handle("/metrics", promhttp.Handler())
	fmt.Println("Startet Prometheus Endpoint on http://localhost:4444")
	http.ListenAndServe(":4444", nil)
}
