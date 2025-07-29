FROM alpine:latest

RUN apk add --no-cache curl jq openssl

WORKDIR /scripts
COPY init-vault/vault-init.sh /scripts/vault-init.sh
RUN chmod +x /scripts/vault-init.sh

RUN wget -O /tmp/vault.zip https://releases.hashicorp.com/vault/1.15.4/vault_1.15.4_linux_amd64.zip \
    && unzip /tmp/vault.zip -d /usr/local/bin \
    && rm /tmp/vault.zip \
    && chmod +x /usr/local/bin/vault

CMD ["/scripts/vault-init.sh"]