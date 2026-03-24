# Build — Qt5 / qmake (Windows)

## Pré-requis
- Qt 5.12+ (recommandé 5.15.x) avec modules **Widgets** et **Sql**
- Un kit de compilation (MSVC ou MinGW) correspondant à votre Qt

## Compilation
Dans une invite correspondant à votre kit Qt (ex: "Qt 5.15.2 (MSVC 2019 64-bit)") :

```bat
qmake MySQLAccessTester.pro
nmake
```

Ou avec MinGW :

```bat
qmake MySQLAccessTester.pro
mingw32-make
```

L'exécutable est généré dans `bin/`.

## Déploiement runtime (très important)
### 1) Qt DLLs
Utilisez `windeployqt` sur l'exécutable :

```bat
windeployqt bin\MySQLAccessTester.exe
```

### 2) Driver MySQL pour Qt (QMYSQL)
- Copiez `qsqlmysql.dll` dans `bin\sqldrivers\` (ou `bin\plugins\sqldrivers\` selon déploiement)
- Copiez `libmysql.dll` à côté de l'exe (`bin\`), ou assurez-vous qu'elle est dans le PATH
