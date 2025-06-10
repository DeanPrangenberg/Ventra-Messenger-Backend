# Build-Stage: Abhängigkeiten und Build-Tools
FROM debian:stable-slim AS builder

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    libssl-dev \
    qt6-base-dev \
    qt6-websockets-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /build

COPY Backend/VM-REDIS Backend/VM-REDIS
COPY Shared Shared

WORKDIR /build/Backend/VM-REDIS

RUN cmake -S . -B build && cmake --build build --target VM-REDIS && strip build/VM-REDIS

# Laufzeit-Stage: Nur Laufzeitabhängigkeiten
FROM debian:stable-slim

RUN apt-get update && apt-get install -y \
    libstdc++6 \
    libssl3 \
    libqt6core6 \
    libqt6network6 \
    libqt6websockets6 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /build/Backend/VM-REDIS/build/VM-REDIS .

EXPOSE 8881

CMD ["./VM-REDIS"]
