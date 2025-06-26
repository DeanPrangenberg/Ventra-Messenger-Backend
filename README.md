## **Allgemein**

**Ventra-Messenger** ist eine Kombination aus den Funktionen von Discord und WhatsApp. Nutzer sollen wie bei WhatsApp mit Kontakten und Gruppen kommunizieren können, während gleichzeitig Community-Chatserver nach dem Vorbild von Discord angeboten werden.

Die Anwendung soll plattformübergreifend nutzbar sein (Windows, Linux, Android, iOS und Web). Das **Frontend** wird mit C++ und Qt entwickelt, da diese Kombination auf vielen Geräten direkt unterstützt wird. Das **Backend** basiert auf Go sowie Docker und evtl. kubernetes. Docker dient dazu, das Backend in fünf Bereiche zu unterteilen.

## **Chat-Modelle**

Es werden verschiedene Arten der Backend-Kommunikation angeboten:
1. Normale Verbindung zum öffentlichen Backend über das Clear Web.
2. Verbindung zum öffentlichen Backend über das Dark Web mittels TOR, um mehr Sicherheit zu bieten, ohne selbst hosten zu müssen.
3. Verbindung zu einem selbst gehosteten Backend über das Clear Web.
4. Verbindung zu einem selbst gehosteten Backend über das Dark Web.
5. Direkte IP-Kommunikation über das Clear Web ohne Server (nur für Privatchats zwischen zwei Personen).
6. Direkte IP-Kommunikation über das Dark Web.
7. Ein spezielles Pool-Modell: Ein einzelner Channel wird gefiltert basierend auf einem vereinbarten Schlüssel und Filter-String. Der Filter-String bestimmt, welche Nachrichten angezeigt werden, während der Schlüssel die Nachrichten im Pool verschlüsselt. Dieses Modell wird ausschließlich im Dark Web gehostet.

## **Verschlüsselung**

Die Verschlüsselung erfolgt (bei unterstützten Chatmodellen) ausschließlich lokal auf den Geräten der Nutzer. Es wird der **Double Ratchet Algorithmus** als Grundlage verwendet, der eine Ende-zu-Ende-Verschlüsselung mit hoher Sicherheit und zusätzlichen Vorteilen bietet. Der Algorithmus kombiniert zwei Mechanismen: eine **Diffie-Hellman-Ratsche**, die regelmäßig neue Schlüssel durch elliptische Kurven-Kryptographie (ECDH) generiert, und eine **Hash-Ratsche**, die für jede Nachricht neue Schlüssel ableitet.

Der Double Ratchet Algorithmus sorgt für wichtige Sicherheitsmerkmale wie **Forward Secrecy** (frühere Nachrichten bleiben auch bei einem kompromittierten Schlüssel sicher) und **Post-Compromise Security** (nach einer Kompromittierung wird die Sicherheit automatisch wiederhergestellt). Zudem ermöglicht er asynchrone Kommunikation, sodass Nachrichten auch dann sicher zugestellt werden können, wenn ein Teilnehmer offline ist.
