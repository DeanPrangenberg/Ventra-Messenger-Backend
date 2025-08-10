listener "tcp" {
  address     = "0.0.0.0:8200"
  tls_disable = 1    # Prod TLS ACTIVE
}

storage "raft" {
  path    = "/vault/data"
  node_id = "transit-vault-0"
}

api_addr = "http://transit-vault.default.svc.cluster.local:8200"
cluster_addr = "http://transit-vault.default.svc.cluster.local:8201"

seal "shamir" {

}
