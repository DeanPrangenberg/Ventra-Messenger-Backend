FROM fedora:40

RUN dnf install -y \
    libstdc++ \
    openssl \
    qt6-qtbase-devel \
    qt6-qtwebsockets-devel && \
    dnf clean all

WORKDIR /app

COPY VM-LOGIC .

RUN chmod +x /app/VM-LOGIC && ldd /app/VM-LOGIC || true

EXPOSE 8001

CMD ["./VM-LOGIC"]