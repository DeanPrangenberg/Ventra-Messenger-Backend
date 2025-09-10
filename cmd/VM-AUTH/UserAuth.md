# UserAuth
### Anfragen
##### Init
1. User generiert schlüsselpaar
2. User sendet PublicKey über 'PublicKeyRequest'
3. Server generiert Token und sendet diesen verschlüsselt mit dem PublicKey zurück
4. Server speichert client public key von client und das shared secret in der redis

##### Register
1. User läd seinen in der Init generierten schlüssel
2. User sendet 'RegisterRequest' mit den User Daten sowie dem Schlüssel
3. Server validiert die Daten und läd die Daten aus der Redis
4. Server generiert UserID, Session u. Long Time Token
5. Server sendet die Daten verschlüsselt mit dem PublicKey zurück
6. Server löscht die Daten in der Redis

##### Login
1. User läd seinen in der Init generierten schlüssel
2. User sendet 'LoginRequest' mit den User Daten sowie dem Schlüssel
3. Server validiert die Daten und läd die Daten aus der Redis
4. Server generiert UserID, Session u. Long Time Token
5. Server sendet die Daten verschlüsselt mit dem PublicKey zurück
6. Server löscht die Daten in der Redis

