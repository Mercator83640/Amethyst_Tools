# PortInspector (Qt 5.15 / MSVC 2019 x64)

Petit outil GUI pour tester un port TCP.

## Tests disponibles
- **Check Local** : vérifie si le port est libre localement (bind/listen).
- **Check Remote** : vérifie si le port est joignable sur `Host/IP:Port` (connect TCP).
- **Verdict global** : combine les deux selon le mode:
  - **Serveur** : OK si le port est libre localement.
  - **Client** : OK si la connexion distante réussit.
  - **Both**   : OK si local libre ET distant ouvert.

## Build (qmake + nmake)
Ouvrir un "x64 Native Tools Command Prompt" :

```bat
cd PortInspector
C:\Qt\5.15.2\msvc2019_64\bin\qmake.exe PortInspector.pro -spec win32-msvc "CONFIG+=release"
nmake
```
