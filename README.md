# Messenger-App Architektur

## Übersicht
Dieses Dokument beschreibt die **Container-Struktur** des Backends und die **Module** des Frontends für einen Messenger-Dienst, der mit Docker containerisiert ist.

---

## Backend (Server)

### **Container 1: API-Gateway**
**Zuständigkeit**:
- Zentrale Anlaufstelle für alle eingehenden Anfragen vom Frontend.
- Routet Anfragen an die richtigen Services (Auth, Chat, etc.).
- Blockiert nicht autorisierte Zugriffe (z. B. ohne gültiges JWT-Token).

**Technologie**:
- Nginx oder Traefik (als Reverse-Proxy).
- Konfiguration: [Siehe `nginx.conf`](#docker-compose-setup).

---

### **Container 2: Datenbank-Speicher**
**Zuständigkeit**:
- Speichert **Benutzerdaten** (Passwörter gehasht + gesalzen).
- Verwaltet **Chat-Metadaten** (Chat-Räume, Nachrichten-Zeitstempel).

**Technologie**:
- PostgreSQL (für strukturierte Daten).
- Persistente Speicherung via Docker-Volumes.

---

### **Container 3: Auth-Service**
**Zuständigkeit**:
- Registrierung, Login und Token-Generierung (JWT).
- Überprüfung von Benutzerberechtigungen.

**Technologie**:
- Node.js (Express) oder Python (FastAPI).
- Kommuniziert mit der PostgreSQL-Datenbank.

---

### **Container 4: Chat-Service**
**Zuständigkeit**:
- Verarbeitet **Nachrichten in Echtzeit** über WebSockets.
- Speichert Chat-Verläufe in der Datenbank.
- Nutzt Redis für temporäre Caching (z. B. aktive Sessions).

**Technologie**:
- Socket.io (WebSockets) + Node.js.
- Integration mit Redis für schnelle Zugriffe.

---

### **Container 5: Cache**
**Zuständigkeit**:
- Speichert **temporäre Daten** (z. B. aktive Benutzer, Chat-Sitzungen).
- Entlastet die Hauptdatenbank durch schnelle Zwischenspeicherung.

**Technologie**:
- Redis (In-Memory-Datenbank).

---

## Frontend (Client)

### **Module**
1. **Encryption / Decryption**
    - Verschlüsselt Nachrichten **clientseitig** (z. B. mit AES).
    - Schlüsselaustausch via Diffie-Hellman (für E2E-Verschlüsselung).

2. **Lokaler Speicher**
    - Speichert Chats offline (via `localStorage` oder IndexedDB).
    - Synchronisiert mit dem Server, sobald online.

3. **GUI**
    - **Chats**: Liste der Konversationen + Nachrichtenverlauf.
    - **Settings**: Benutzerprofil, Passwortänderung, Benachrichtigungen.
    - **Navigation Bar**: Schnellzugriff auf Chats, Kontakte, Einstellungen.

4. **Server-Kommunikation**
    - Verbindet zum API-Gateway via **HTTP/REST** (für Profildaten).
    - Nutzt **WebSockets** für Echtzeit-Chats.

5. **Direkte Client-Kommunikation**
    - Peer-to-Peer-Verbindungen (optional, z. B. mit WebRTC für Dateitransfers).

6. **App Settings**
    - Theme-Auswahl (Dark/Light Mode).
    - Benachrichtigungseinstellungen (Push/Desktop).
