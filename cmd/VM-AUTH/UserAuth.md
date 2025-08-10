# UserAuth
```mermaid
graph TD
    subgraph User["Desktop Client (C++/Qt)"]
        UI[User Login]
    end
    
    UI <--> |Send Username und Password-Hash via HTTP| API
    
    subgraph Backend["Backend Services"]
        API[Api]
        LOGGER[Logger]
        AUTH[Auth]
        CORE[Core]

        API --> |Validate Login Data with DB| CORE
        API --> |Send new Tokens| UI

        CORE <--> |Get new Refresh and Session JWT| AUTH
        CORE --> |Send new Tokens| API

        AUTH --> |Log new Token Statistics| LOGGER
    end
```

# SeviceAUTH
- Beim start des Backend machten die AUTH Services eine anfrage an den HashiVault um zu gucken ob es schon ein Root CA gibt, wenn ja holen sie sich es und melden isch beim core service der trägt sie dann als Auth service ein, wenn nicht melden sie sich auch beim core service der gibt dann den befehel an den Ersten sich meldenden ein Root CA zu erstellen das root ca wird dann im Vault gesecihert und die andern AUTH instanzen bekommen eine anfrage sich neu zustarten und das CA auf zunehmen.
- Nun werden vom Head AUTH service für alle eingetragende Services zertifikate erstellt so das alles auf mTLS umsteigen kann also für alle API, Message Dispatcher, Kafka, Core, Logger, Auth und Redis-API instanzen
