QT       += widgets sql network
CONFIG   += c++17
TEMPLATE = app
TARGET   = MySQLAccessTester

SOURCES += \
    src/main.cpp \
    src/MainWindow.cpp

HEADERS += \
    src/MainWindow.h

FORMS += \
    src/MainWindow.ui

RESOURCES += \
    resources/assets.qrc

# Windows application icon (compiled resource)
win32:RC_FILE = resources/app.rc

# Optional: place built binary in ./bin
DESTDIR = bin
