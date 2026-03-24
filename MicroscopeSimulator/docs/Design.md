# Documentation de conception — Microscope Simulator

## But
Simuler l'application Acquisition (partie communication) afin de :
1) recevoir les informations de plaque envoyées par Schedule,
2) simuler l'état microscope (FREE/OCCUPIED/RELEASE) renvoyé à Schedule.

## Interfaces réseau
### Plaques (entrée)
- UDP port 10332
- Payload: JSON compact
  - `CB` (string)
  - `Date` (ISO `YYYY-MM-DD`)
  - `DualAnalysis` (bool)

### Status (sortie)
- UDP port 12345 vers host configurable (défaut 127.0.0.1)
- Payload: ASCII `"0"`, `"1"`, `"2"`.

## Machine d'état
- Initial: FREE
- OnPlateReceived: OCCUPIED
- OnReleaseButton: RELEASE puis retour automatique à FREE après 10 secondes

## Robustesse
- Réception UDP: lecture en boucle `while(hasPendingDatagrams())`
- Envoi status périodique (optionnel) pour limiter l'impact de la perte UDP.


## ACK (acquittement)
- Entrée UDP (port 10333 par défaut) pour recevoir un acquittement de Schedule.
- Formats acceptés : `ACK`, `ACK_RELEASE`, ou JSON `{"type":"ack","for":"release"}`.
- Effet : si l'état courant est RELEASE, l'ACK provoque le retour immédiat à FREE.
- Fallback : sans ACK, retour FREE automatique après 10 secondes.
