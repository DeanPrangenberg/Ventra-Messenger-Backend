FROM fedora:40

RUN dnf install -y \
    libstdc++ \
    openssl \
    qt6-qtbase-devel \
    qt6-qtwebsockets-devel && \
    dnf clean all

WORKDIR /app

COPY VM-AUTH .

RUN chmod +x /app/VM-AUTH && ldd /app/VM-AUTH || true

EXPOSE 8001

CMD ["./VM-AUTH"]