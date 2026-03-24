QT += widgets network
CONFIG += c++17
TEMPLATE = app
TARGET = MicroscopeSimulator

SOURCES += \
    src/main.cpp \
    src/MainWindow.cpp

HEADERS += \
    src/MainWindow.h

FORMS += \
    src/MainWindow.ui

RESOURCES += \
    resources/assets.qrc

win32:RC_FILE = resources/app.rc

DESTDIR = bin
