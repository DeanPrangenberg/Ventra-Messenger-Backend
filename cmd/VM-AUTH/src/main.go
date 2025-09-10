package main

import (
	"VM-AUTH/src/PrometheusEndpoint"
	gRPCserver "VM-AUTH/src/gRPC-Server"
)

func main() {
	go gRPCserver.StartGRPCServer()
	go PrometheusEndpoint.StartPrometheusEndpoint()

	/*
		tokenManager, err := JWT-Tokens.NewTokenManager(config)
		if err != nil {
			log.Fatalf("Fehler beim Erstellen des TokenManagers: %v", err)
		}

		userID := "user123"
		fmt.Println("=== JWT Token Management mit Ausfallsicherheit ===\n")

		fmt.Println("1. Erstelle Long-Time-Token...")
		longTimeToken, err := tokenManager.CreateRefreshToken(userID)
		if err != nil {
			log.Fatal("Fehler beim Erstellen des Long-Time-Tokens:", err)
		}
		fmt.Println("✓ Long-Time-Token erstellt")

		fmt.Println("\n2. Verifiziere Long-Time-Token...")
		longClaims, err := tokenManager.VerifyToken(longTimeToken)
		if err != nil {
			log.Fatal("Fehler bei der Verifikation des Long-Time-Tokens:", err)
		}
		fmt.Printf("✓ Long-Time-Token verifiziert - UserID: %s, Typ: %s\n", longClaims.UserID, longClaims.TokenType)

		fmt.Println("\n3. Erstelle Session-Token aus Long-Time-Token...")
		sessionToken, err := tokenManager.CreateSessionToken(longTimeToken)
		if err != nil {
			log.Fatal("Fehler beim Erstellen des Session-Tokens:", err)
		}
		fmt.Println("✓ Session-Token erstellt")

		fmt.Println("\n4. Verifiziere Session-Token...")
		sessionClaims, err := tokenManager.VerifyToken(sessionToken)
		if err != nil {
			log.Fatal("Fehler bei der Verifikation des Session-Tokens:", err)
		}
		fmt.Printf("✓ Session-Token verifiziert - UserID: %s, Typ: %s\n", sessionClaims.UserID, sessionClaims.TokenType)

		fmt.Println("\n5. Refresh Session-Token...")
		refreshedToken, err := tokenManager.RefreshSessionToken(sessionToken)
		if err != nil {
			log.Fatal("Fehler beim Refresh des Session-Tokens:", err)
		}
		fmt.Println("✓ Session-Token refreshed")

		fmt.Println("\n6. Sperre Token...")
		err = tokenManager.RevokeToken(refreshedToken)
		if err != nil {
			log.Fatal("Fehler beim Sperren des Tokens:", err)
		}
		fmt.Println("✓ Token gesperrt")

		fmt.Println("\n7. Teste gesperrten Token...")
		_, err = tokenManager.VerifyToken(refreshedToken)
		if err != nil {
			fmt.Printf("✓ Gesperrter Token korrekt abgelehnt: %v\n", err)
		}

		fmt.Println("\n=== Token Informationen ===")
		fmt.Printf("Long-Time-Token Gültigkeit: %v Tage\n", longClaims.ExpiresAt.Sub(longClaims.IssuedAt.Time).Hours()/24)
		fmt.Printf("Session-Token Gültigkeit: %v Minuten\n", sessionClaims.ExpiresAt.Sub(sessionClaims.IssuedAt.Time).Minutes())

		fmt.Println("\n=== Ausfallsicherheit ===")
		fmt.Println("✓ Token-Informationen werden in DB gespeichert")
		fmt.Println("✓ Tokens können gesperrt werden")
		fmt.Println("✓ Tokens werden auf Gültigkeit geprüft")
		fmt.Println("✓ Parent-Child Beziehungen werden verwaltet")
	*/
	select {}
}
