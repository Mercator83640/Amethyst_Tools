QT += core
CONFIG += console c++17
CONFIG -= app_bundle

TEMPLATE = app
TARGET = HyperFocusConsoleTest

SOURCES += \
    main.cpp \
    cimagefilter.cpp

HEADERS += \
    cimagefilter.h

# Adjust these paths to your OpenCV installation
INCLUDEPATH += C:\adretek\AdReTek_Projects\Amethyst\third_party\OpenCV\include
LIBS += -LC:\adretek\AdReTek_Projects\Amethyst\third_party\OpenCV\lib

# Example OpenCV libs (adapt to your version)
LIBS += -lopencv_world460

LIBS += -LC:\adretek\AdReTek_Projects\Amethyst\ResultatBin\x64\Release\_libs
LIBS += -lLib_HyperFocalTreatment


LIBS += -luser32
