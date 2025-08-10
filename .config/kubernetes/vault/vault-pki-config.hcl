ui = true
disable_mlock = true

storage "file" {
  path = "/vault/data"
}

listener "tcp" {
  address = "0.0.0.0:8200"
  tls_disable = "true"
}

seal "transit" {
  address = "http://transit-vault.vault.svc.cluster.local:8200"
  disable_renewal = "false"
  key_name = "autounseal"
  mount_path = "transit/"
  tls_skip_verify = "true"
  token = "____autounseal-token.txt____"
}

api_addr = "http://pki-vault.vault.svc.cluster.local:8200"