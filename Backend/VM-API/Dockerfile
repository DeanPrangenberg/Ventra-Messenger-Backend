FROM fedora:40 AS builder

RUN dnf install -y \
    libstdc++ \
    openssl \
    qt6-qtbase-devel \
    qt6-qtwebsockets-devel \
    cmake \
    gcc-c++ \
    make && \
    dnf clean all

WORKDIR /build

COPY . .

WORKDIR /build/Backend/VM-API

RUN cmake -S . -B build && cmake --build build --target VM-API

FROM fedora:40

RUN dnf install -y \
    libstdc++ \
    openssl \
    qt6-qtbase \
    qt6-qtwebsockets && \
    dnf clean all

WORKDIR /app

COPY --from=builder /build/Backend/VM-API/build/VM-API .

EXPOSE 8881

CMD ["./VM-API"]