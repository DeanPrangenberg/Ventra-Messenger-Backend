FROM fedora:40

RUN dnf install -y \
    libstdc++ \
    openssl \
    qt6-qtbase-devel \
    qt6-qtwebsockets-devel && \
    dnf clean all

WORKDIR /app

COPY VM-REDIS .

RUN chmod +x /app/VM-REDIS && ldd /app/VM-REDIS || true

EXPOSE 8001

CMD ["./VM-REDIS"]