FROM jetbrains/qodana-cpp:2025.1

RUN apt-get update && \
    apt-get install -y \
      qt6-base-dev qt6-websockets-dev libssl-dev sqlcipher libsqlcipher-dev pkg-config
