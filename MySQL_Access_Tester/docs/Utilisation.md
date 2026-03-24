# Utilisation — MySQL Access Tester

## 1) Côté serveur (WAMP / MySQL 8)
Pour qu’un poste du LAN puisse se connecter :

1. **MySQL écoute sur le réseau**
   - vérifier `bind-address` dans `my.ini`
   - éviter `127.0.0.1` uniquement

2. **Pare-feu Windows**
   - autoriser TCP entrant sur le port MySQL (souvent **3306**)

3. **Utilisateur MySQL autorisé depuis le LAN**
   Exemple (à adapter) :
   ```sql
   CREATE USER 'test_lan'@'10.0.0.%' IDENTIFIED BY 'MotDePasseSolide!';
   GRANT SELECT ON ma_base.* TO 'test_lan'@'10.0.0.%';
   FLUSH PRIVILEGES;
   ```

## 2) Côté client (l’application)
1. Renseigner l’IP (ex: `10.0.0.12`) et le port (par défaut 3306)
2. Renseigner base / user / password
3. Cliquer **Tester l’accès**

### Interprétation
- **OK** : connexion réussie et (optionnellement) `SELECT 1` OK
- **KO connexion** : réseau/pare-feu/port/mauvais user/pass
- **KO requête** : connexion OK, mais droits SQL insuffisants ou base invalide

## 3) Si QMYSQL n’apparaît pas dans “Drivers Qt SQL disponibles”
C’est un problème de driver/plugin Qt SQL.

À vérifier :
- Le plugin `qsqlmysql.dll` est bien présent (souvent dans `plugins/sqldrivers/` près de l’exe)
- `libmysql.dll` est présent à côté de l’exe (ou accessible via PATH)
- Les versions (compiler/ABI) sont compatibles (même famille MSVC/Qt)

Astuce : copiez le dossier `plugins/` près de l’exe, puis assurez-vous que `plugins/sqldrivers/qsqlmysql.dll` existe.
