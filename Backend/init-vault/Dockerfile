# init-vault/Dockerfile
FROM hashicorp/vault:1.20.1

# Install necessary tools
RUN apk add --no-cache curl jq bash

# Copy the initialization script
COPY vault-init-auto.sh /usr/local/bin/
RUN chmod +x /usr/local/bin/vault-init-auto.sh

# Set the script as the entrypoint
ENTRYPOINT ["/usr/local/bin/vault-init-auto.sh"]