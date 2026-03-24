QT += core gui widgets concurrent
CONFIG += c++17
TEMPLATE = app
TARGET = HyperFocalTester_Enhanced

SOURCES += \
    logger.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    logger.h \
    mainwindow.h

# ---- Adjust these paths in your environment ----
 INCLUDEPATH += C:/adretek/AdReTek_Projects/Amethyst/third_party/OpenCV/include
 LIBS += -LC:/adretek/AdReTek_Projects/Amethyst/third_party/OpenCV/lib
 LIBS += -lopencv_world460

INCLUDEPATH += C:/adretek/AdReTek_Projects/Amethyst/libs/Lib_HyperFocalTreatment/include
LIBS += -LC:/adretek/AdReTek_Projects/Amethyst/ResultatBin/x64/Release/_libs -lLib_HyperFocalTreatment
