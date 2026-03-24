QT += core gui widgets serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
TEMPLATE = app
TARGET = CytomatSimulator

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    cytomat_engine.cpp \
    serial_server.cpp

HEADERS += \
    mainwindow.h \
    cytomat_types.h \
    cytomat_engine.h \
    serial_server.h

# Uncomment to add a Windows executable icon
# RC_FILE = resources/app.rc
