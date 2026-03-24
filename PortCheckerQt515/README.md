# PortChecker (Qt 5.15.2 / MSVC 2019)

Application Qt Widgets qui :
- **Local** : teste si un port TCP est **libre** sur la machine (en tentant `QTcpServer::listen`).
- **Distance** : teste si un port TCP est **ouvert** sur un hôte distant (en tentant `QTcpSocket::connectToHost`).

## Build
- Qt 5.15.2 (MSVC 2019)
- qmake : `QT += widgets network`

Ouvrir `PortChecker.pro` dans Qt Creator, choisir le kit **MSVC 2019 64-bit**, Build & Run.

## Notes importantes
- À distance, un client ne peut pas prouver qu'un port est "libre" sur l'hôte distant ; il peut seulement déterminer s'il est **ouvert (un service écoute)** ou **non ouvert** (refus/timeout/filtré).
