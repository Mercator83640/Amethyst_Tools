# Microscope Simulator (Qt5 / UDP)

Simulateur pour remplacer l'application Acquisition pendant les tests.

## Comportement
- Status par défaut : **FREE (0)**
- Le simulateur **écoute les plaques** envoyées par Schedule sur **UDP 10332** (JSON)
- Quand une plaque est reçue : il affiche CB/Date/DualAnalysis et passe automatiquement en **OCCUPIED (1)**, puis envoie le status sur **UDP 12345**
- Un bouton **RELEASE** permet d'envoyer **RELEASE (2)** pour libérer la plaque (Schedule peut reprendre)

## Ports par défaut
- Réception plaques : UDP **10332**
- Envoi status : UDP **12345** vers `127.0.0.1`

## Build (Qt5 / qmake)
```bat
qmake MicroscopeSimulator.pro
nmake   (ou mingw32-make)
```

Exécutable : `bin\MicroscopeSimulator.exe`

## Notes
- Bind plaques : `AnyIPv4` est recommandé (capte broadcast + localhost). Si vous ne testez qu'en local, `LocalHost` suffit.
- UDP n'est pas fiable : option "Répéter l’envoi du status" pour éviter une perte de datagram.


## Nouvelle règle
Après un clic sur RELEASE, le simulateur repasse automatiquement en FREE après 10 secondes.


## ACK (acquittement)
Le simulateur écoute un acquittement UDP sur le port **10333** (configurable). Après RELEASE, il attend un message `ACK_RELEASE` (ou `ACK`, ou JSON `{type:"ack", for:"release"}`) pour repasser immédiatement à FREE. Sans ACK, retour FREE après 10s (fallback).
