FROM jetbrains/qodana-cpp:2025.1

RUN apt-get update && \
    apt-get install -y \
      qt6-base-dev qt6-base-dev-tools qt6-websockets-dev qt6-gui-dev qt6-widgets-dev \
      libssl-dev libsqlite3-dev sqlcipher pkg-config