# 🔐 Ventra-Messenger - Secure & Scalable Self-Hosted Messenger (still in development)

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)
![Docker](https://img.shields.io/badge/docker-ready-success)
![Go Version](https://img.shields.io/badge/go-1.20%2B-blue)
![C++](https://img.shields.io/badge/C%2B%2B-17%2B-orange)

A high-performance messenger with end-to-end encryption, scalable cloud architecture, and full self-hosting capability.

```mermaid
graph TD
    subgraph Client["Desktop Client (C++/Qt)"]
        UI[Qt Client UI]
    end
    
    UI <--> |Webscoket|API[API]
    
    subgraph Backend["Backend Services"]
        AUTH <-->|Publish/Subscribe| KAFKA[Kafka]
        API <-->|Publish/Subscribe| KAFKA
        CS[CoreService] -->|Process| POSTGRES[(PostgreSQL)]
        CS -->|Cache| REDISAPI[REDIS-API]
        REDIS[(Redis)] <-->|Process| REDISAPI
        CS <-->|Publish/Subscribe| KAFKA
        KAFKA <-->|Publish/Subscribe| MD[Message Dispatcher]
        KAFKA <-->|Certs| CM[Cert-Manager]
        PV[Pki Vault] -->|Certs| CM
        TV[Transit Vault] -->|unseal| PV
        MD -->|Cache| REDISAPI
        LOGGER[Logger Service] -->|Expose| PROM[Prometheus]
        LOGGER -->|Log| REDISAPI
        API -->|Log| LOGGER
        CS -->|Log| LOGGER
        MD -->|Log| LOGGER
        PROM -->|Scrape| PV
        GRAF[Grafana] --> |Display| PROM 
    end
```

## 🌟 Key Features
- **Military-Grade Encryption**: Double Ratchet + OpenSSL for E2E encryption with Forward Secrecy
- **High-Performance Backend**: Parallelized Go services with Kafka
- **Native Desktop Client**: Resource-efficient Qt/C++ app with local SQLite database
- **Enterprise Scalability**: Horizontal scaling with Redis and Kafka

# 🧱 Technology Stack
### Component	Technologies
- **Backend**:	Go, Docker
- **Frontend**:	C++17, Qt 6, SQLite3 + SQLCipher
- **Realtime**:	WebSockets, Kafka, Redis Pub/Sub
- **Data Storage**:	PostgreSQL, Redis, SQLite
- **Security**:	OpenSSL, Double Ratchet (AES-256-GCM + X25519 + HKDF), JWT, Ed25519,
- **Infrastructure**:	Docker Compose, Kubernetes

# 🚀 Local Installation
### Backend
```bash
not ready
``` 

#### Access Points:
- not ready

### Client:
```bash
not ready
```
