FROM fedora:40

RUN dnf install -y \
    libstdc++ \
    openssl \
    qt6-qtbase-devel \
    qt6-qtwebsockets-devel && \
    dnf clean all

WORKDIR /app

COPY VM-MAIN-DB .

RUN chmod +x /app/VM-MAIN-DB && ldd /app/VM-MAIN-DB || true

EXPOSE 8001

CMD ["./VM-MAIN-DB"]