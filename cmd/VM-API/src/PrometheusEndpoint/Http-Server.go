package PrometheusEndpoint

import (
	"fmt"
	"net/http"

	"github.com/prometheus/client_golang/prometheus"
	"github.com/prometheus/client_golang/prometheus/promhttp"
)

func initMetrics() {
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

/*
 * Gauge:
 * PrometheusEndpoint.ConnectedClients.Inc()
 *
 */
func StartPrometheusEndpoint() {
	initMetrics()
	http.Handle("/metrics", promhttp.Handler())
	fmt.Println("Startet Prometheus Endpoint on http://localhost:4444")
	http.ListenAndServe(":4444", nil)

}
