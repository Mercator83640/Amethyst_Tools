# Created by and for Qt Creator This file was created for editing the project sources only.
# You may attempt to use it for building too, by modifying this file here.

#TARGET = SampleFocusStable

QT = core gui widgets
CXXFLAGS += /D_USE_MATH_DEFINES

HEADERS = \
   $$PWD/src/fast_bilateral.hh \
   $$PWD/src/focusstack.hh \
   $$PWD/src/histogrampercentile.hh \
   $$PWD/src/logger.hh \
   $$PWD/src/options.hh \
   $$PWD/src/radialfilter.hh \
   $$PWD/src/task_3dpreview.hh \
   $$PWD/src/task_align.hh \
   $$PWD/src/task_background_removal.hh \
   $$PWD/src/task_denoise.hh \
   $$PWD/src/task_depthmap.hh \
   $$PWD/src/task_depthmap_inpaint.hh \
   $$PWD/src/task_focusmeasure.hh \
   $$PWD/src/task_grayscale.hh \
   $$PWD/src/task_loadimg.hh \
   $$PWD/src/task_merge.hh \
   $$PWD/src/task_reassign.hh \
   $$PWD/src/task_saveimg.hh \
   $$PWD/src/task_wavelet.hh \
   $$PWD/src/task_wavelet_opencl.hh \
   $$PWD/src/task_wavelet_templates.hh \
   $$PWD/src/worker.hh

SOURCES = \
   $$PWD/src/focusstack.cc \
  $$PWD/src/histogrampercentile.cc \
   $$PWD/src/logger.cc \
   $$PWD/src/main.cc \
   $$PWD/src/options.cc \
   $$PWD/src/radialfilter.cc \
   $$PWD/src/radialfilter_tests.cc \
   $$PWD/src/task_3dpreview.cc \
   $$PWD/src/task_align.cc \
   $$PWD/src/task_background_removal.cc \
   $$PWD/src/task_denoise.cc \
   $$PWD/src/task_depthmap.cc \
   $$PWD/src/task_depthmap_inpaint.cc \
   $$PWD/src/task_focusmeasure.cc \
   $$PWD/src/task_grayscale.cc \
   $$PWD/src/task_grayscale_tests.cc \
   $$PWD/src/task_loadimg.cc \
   $$PWD/src/task_merge.cc \
   $$PWD/src/task_reassign.cc \
   $$PWD/src/task_saveimg.cc \
   $$PWD/src/task_wavelet.cc \
   $$PWD/src/task_wavelet_opencl.cc \
   $$PWD/src/task_wavelet_opencl_tests.cc \
   $$PWD/src/task_wavelet_tests.cc \
   $$PWD/src/worker.cc

INCLUDEPATH =

#DEFINES = 

# Adjust these paths to your OpenCV installation
INCLUDEPATH += C:\adretek\AdReTek_Projects\Amethyst\third_party\OpenCV\include
LIBS += -LC:\adretek\AdReTek_Projects\Amethyst\third_party\OpenCV\lib
