listener "tcp" {
  address     = "0.0.0.0:8200"
  tls_disable = 1    # Für Test; in Produktion TLS aktivieren!
}

storage "raft" {
  path    = "/vault/data"
  node_id = "transit-vault-0"
}

api_addr = "http://transit-vault.default.svc.cluster.local:8200"
cluster_addr = "http://transit-vault.default.svc.cluster.local:8201"

seal "shamir" {
  # Kein Auto-Unseal, muss manuell init/unseal erfolgen
}
