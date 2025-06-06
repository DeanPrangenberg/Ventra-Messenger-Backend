FROM fedora:40

RUN dnf install -y \
    libstdc++ \
    openssl \
    qt6-qtbase-devel \
    qt6-qtwebsockets-devel && \
    dnf clean all

WORKDIR /app

COPY VM-API .

RUN chmod +x /app/VM-API && ldd /app/VM-API || true

EXPOSE 8001

CMD ["./VM-API"]