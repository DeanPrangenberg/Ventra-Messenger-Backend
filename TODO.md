# 📋 Development TODOs

## 🔄 Shared Components & Wrappers
- [ ] 🧪 Add unit tests and documentation (info.md) to Kafka-Wrapper
- [x] (25.07.2025) 📦 Write the REDIS-API-Wrapper
- [ ] 📦 Write the Vault-Wrapper
- [ ] 📦 Write the AUTH-Wrapper

## AUTH Service
- [ ] 📝 Add unit tests and info.md
- [ ] 📝 Write JWT functions (jwt-gen and jwt-verify)
- [ ] 📝 Write mTLS system
  - [ ] 📝 Functions for CA generation
  - [ ] 📝 Functions for CA Validation / signing
  - [ ] 📝 Functions for CA update
- [ ] 📝 Write Logging events for Logger Service
- [ ] 📝 Write new gRPC-Server with mTLS and JWT requests
- [ ] 📝 Write AUTH-API-Wrapper for gRPC-Server

## 🏗️ Core Services (Self-Made)
- [ ] 📐 Plan + Rewrite API with unit tests and info.md
- [ ] 📐 Plan Logger, CORE and AUTH Service + Refine Kafka integration
- [ ] 📝 Write Logger Service
- [ ] 📝 Write AUTH Service
- [ ] 📝 Write CORE Service

## 🌐 Infrastructure & Monitoring
- [ ] 📊 Incorporate Prometheus and Grafana for monitoring
- [ ] ☸️ Set up Kubernetes deployment environment
- [ ] 🛠️ Setup HashiCorp Vault and Init script/Container for secrets management
  - [ ] Add Role-based access control (RBAC) for Vault
  - [ ] Add easy way to add easy way to add tokens to Vault
- [ ] 🛠️ Update Kafka to use mTLS + Setup role-based access control (RBAC) for Kafka topics

## Other
- [ ] Integrate mTLS into gRPC-Servers (AUTH and REDIS-API)