storage "file" {
  path = "/.vault/data"
}

listener "tcp" {
  address     = "0.0.0.0:8200"
  tls_cert_file = "/.vault/tls/vault.crt"
  tls_key_file  = "/.vault/tls/vault.key"
  tls_client_ca_file = "/.vault/tls/ca.crt"
}

ui = true
disable_mlock = false
