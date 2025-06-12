FROM jetbrains/qodana-cpp:latest

RUN apt-get update && \
    apt-get install -y \
      qt6-base-dev qt6-base-dev-tools qt6-websockets-dev \
      libssl-dev libsqlite3-dev sqlcipher pkg-config