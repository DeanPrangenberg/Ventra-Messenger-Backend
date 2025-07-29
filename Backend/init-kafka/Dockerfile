FROM alpine:latest

RUN apk add --no-cache curl jq openssl openjdk11-jre \
    && wget -O /tmp/kafka.tgz https://archive.apache.org/dist/kafka/3.7.0/kafka_2.13-3.7.0.tgz \
    && tar -xzf /tmp/kafka.tgz -C /opt \
    && ln -s /opt/kafka_2.13-3.7.0/bin/kafka-topics.sh /usr/local/bin/kafka-topics.sh \
    && rm /tmp/kafka.tgz

RUN wget -O /tmp/vault.zip https://releases.hashicorp.com/vault/1.15.4/vault_1.15.4_linux_amd64.zip \
    && unzip /tmp/vault.zip -d /usr/local/bin \
    && rm /tmp/vault.zip \
    && chmod +x /usr/local/bin/vault

WORKDIR /scripts
COPY init-kafka/kafka-init.sh /scripts/kafka-init.sh
RUN chmod +x /scripts/kafka-init.sh

CMD ["/scripts/kafka-init.sh"]