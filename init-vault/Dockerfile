FROM hashicorp/vault:1.20.1

RUN apk add --no-cache curl jq bash

RUN curl -LO "https://dl.k8s.io/release/$(curl -L -s https://dl.k8s.io/release/stable.txt)/bin/linux/amd64/kubectl" && \
    chmod +x kubectl && \
    mv kubectl /usr/local/bin/

COPY init-vault/vault-init-auto.sh /usr/local/bin/
RUN chmod +x /usr/local/bin/vault-init-auto.sh

ENTRYPOINT ["/usr/local/bin/vault-init-auto.sh"]