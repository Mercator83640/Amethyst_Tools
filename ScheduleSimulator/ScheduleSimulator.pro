QT += core gui network widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ScheduleSimulator
TEMPLATE = app

SOURCES += \
    main.cpp \
    ScheduleSimulator.cpp

HEADERS += \
    ScheduleSimulator.h

FORMS += \
    ScheduleSimulator.ui

CONFIG += c++17

# Uncomment if needed for older MSVC kits
# DEFINES += _CRT_SECURE_NO_WARNINGS

DISTFILES +=

RESOURCES += \
    assets.qrc

RC_FILE = resources/app.rc