# Simple UTC Daemon - Deployment Diagrams

## Basic Deployment Architecture

```mermaid
graph TB
    subgraph "UTC Clients"
        Client1[UTC Client 1]
        Client2[UTC Client 2]
        ClientN[UTC Client N]
    end

    subgraph "UTC Server"
        Server[simple-utcd<br/>Main Process]
        Config[/etc/simple-utcd/<br/>Configuration]
        Logs[/var/log/simple-utcd/<br/>UTC Logs]
    end

    subgraph "Upstream Servers"
        Upstream1[Upstream Server 1]
        Upstream2[Upstream Server 2]
    end

    subgraph "System Services"
        Systemd[systemd<br/>Service Manager]
    end

    Client1 --> Server
    Client2 --> Server
    ClientN --> Server

    Systemd --> Server
    Systemd --> Config

    Server --> Config
    Server --> Logs
    Server --> Upstream1
    Server --> Upstream2
```
