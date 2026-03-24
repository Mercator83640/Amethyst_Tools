QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += C:/adretek/AdReTek_Projects/Amethyst/third_party/OpenCV/include/

LIBS += -L"/adretek/AdReTek_Projects/Amethyst/third_party/OpenCV/"

INCLUDEPATH += C:/adretek/AdReTek_Projects/Portage_Qt/Hyperfocal/Lib_HyperFocalTreatment/

LIBS += -L"c:/adretek/AdReTek_Projects/Portage_Qt/Hyperfocal/build-Lib_HyperFocalTreatment-Desktop_Qt_5_15_2_MSVC2019_64bit-Release/Release"


CONFIG(debug, debug|release) {
LIBS += -l"opencv_world460d"

} else {
LIBS += -l"opencv_world460"

} # CONFIG(debug, debug|release)

LIBS += -l"Lib_HyperFocalTreatment"

SOURCES += \
    cimagefilter.cpp \
    main.cpp \
    imagefilter_mainwindow.cpp

HEADERS += \
    cimagefilter.h \
    imagefilter_mainwindow.h \
    qimagetocvmat.h

FORMS += \
    imagefilter_mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
