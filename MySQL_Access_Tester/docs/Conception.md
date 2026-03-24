# Documentation de conception — MySQL Access Tester

## Objectif
Permettre, depuis n’importe quel PC du réseau local, de vérifier rapidement que :
- le serveur MySQL est joignable sur `IP:port` (par défaut 3306),
- l’utilisateur/mot de passe sont valides,
- et, optionnellement, qu’une requête SQL simple fonctionne (`SELECT 1`).

## Périmètre
- Application **cliente** (elle ne “serve” rien sur 3306)
- Windows + Qt Widgets
- Driver Qt SQL : **QMYSQL**

## Architecture
### UI (Qt Widgets)
- Champs : Host/IP, Port, Database, User, Password, Timeout
- Checkbox : exécuter `SELECT 1`
- Bouton : “Tester l’accès”
- Zone log : traces datées + erreurs

### Couche accès DB
- `QSqlDatabase::addDatabase("QMYSQL")`
- Options de connexion : `MYSQL_OPT_CONNECT_TIMEOUT=<n>`
- Test : `db.open()`, puis `SELECT 1`

### Gestion des connexions Qt SQL (point important)
Chaque test crée une connexion avec un **nom unique** (ex: `healthcheck_<timestamp>`),
puis appelle `QSqlDatabase::removeDatabase(connName)` **après** destruction de l’objet `QSqlDatabase`
(sortie de scope). Cela évite les problèmes de réutilisation/ressources.

## Critères de succès
- Affichage “OK” si la connexion fonctionne (et si `SELECT 1` retourne 1 lorsque coché)
- Affichage “KO” avec message d’erreur Qt SQL (`QSqlError`) sinon

## Points d’attention (QMYSQL)
Le driver **QMYSQL** doit être présent **au runtime** :
- `qsqlmysql.dll` (plugin Qt)
- dépendances (souvent `libmysql.dll`, parfois OpenSSL selon builds)

Sans cela, l’app affichera : “Drivers Qt SQL disponibles: …” sans `QMYSQL`.
