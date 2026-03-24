# MySQL Access Tester (Qt / Windows)

Petit outil Qt (Widgets) pour **tester l’accessibilité** d’un serveur MySQL sur le LAN (ex: WAMP / MySQL 8),
en vérifiant :
1) Connexion TCP + authentification MySQL
2) Optionnel : exécution d’une requête simple `SELECT 1`

## Structure
- `src/` : code source
- `resources/` : icône + ressources Qt
- `docs/` : documentation (conception + utilisation)

## Pré-requis
- Windows 10/11
- **Qt 6.x** avec modules **Widgets** et **Sql**
- Le driver Qt SQL **QMYSQL** disponible au runtime (voir docs)

## Build (CMake)
Exemple (Developer PowerShell for VS) :

```powershell
cmake -S . -B build -G "Ninja" -DCMAKE_PREFIX_PATH="C:\Qt\6.6.2\msvc2019_64"
cmake --build build --config Release
```

L’exécutable se trouve ensuite dans `build/` (ou un sous-dossier selon générateur).
Les docs sont copiées automatiquement dans `.../docs/` à côté de l’exe.

> Si vous n’utilisez pas Ninja, remplacez le générateur (`-G`) par celui que vous avez (Visual Studio, etc.).


## Build (Qt5 / qmake)
Exemple (Qt 5.15.x) :

```powershell
cd .
qmake MySQLAccessTester.pro
mingw32-make   # ou nmake / jom selon votre kit
```

L'exécutable est placé par défaut dans `./bin/`.

> Le module Qt SQL + le plugin `qsqlmysql.dll` doivent être disponibles au runtime (voir docs).
