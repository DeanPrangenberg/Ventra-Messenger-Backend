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

COPY Backend/VM-LOGIC Backend/VM-LOGIC
COPY Shared Shared

WORKDIR /build/Backend/VM-LOGIC

RUN cmake -S . -B build && cmake --build build --target VM-LOGIC && strip build/VM-LOGIC

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

COPY --from=builder /build/Backend/VM-LOGIC/build/VM-LOGIC .

EXPOSE 8881

CMD ["./VM-LOGIC"]
