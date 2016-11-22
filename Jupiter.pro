#-------------------------------------------------
#
# Project created by QtCreator 2015-06-20T15:53:12
# Author Phung Kim Phuong pawnc7@gmail.com
#-------------------------------------------------
#Features:
#Control and process data from HR2D radar
#view ARPA data
#-------------------------------------------------
# Version 3.0.1
#-------------------------------------------------
#data file type defined as .r2d, data starts from 22nd byte of each frame
#run for both 1024 and 1536 data type
#radar resolution can be changed by the user
@CONFIG += debug_and_release@
QT       += core gui
QT       += network
QT   += serialport
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
QMAKE_CXXFLAGS_WARN_OFF -= -Wunused-parameter
TARGET = Jupiter_3.0.1
TEMPLATE = app
SOURCES += main.cpp\
    mainwindow.cpp \
    vnmap.cpp \
    c_arpa_data.cpp \
    dataprocessingthread.cpp \
    qcustombutton.cpp \
    qcustomframe.cpp \
    qcustomcombobox.cpp \
    qcustomgroupbox.cpp \
    qcustomtabwidget.cpp \
    ctarget.cpp \
    CLocal.cpp \
    CTrack.cpp \
    CPoints.cpp \
    CDefine.cpp \
    jtarget.cpp \
    jviewport.cpp \
    jgraphictarget.cpp \
    tinyxml/tinyxml2.cpp \
    c_config.cpp \
    c_radar_data.cpp

HEADERS  += mainwindow.h \
    vnmap.h \
    c_arpa_data.h \
    dataprocessingthread.h \
    qcustombutton.h \
    qcustomframe.h \
    qcustomcombobox.h \
    qcustomgroupbox.h \
    qcustomtabwidget.h \
    pkp.h \
    ctarget.h \
    CLocal.h \
    CTrack.h \
    CPoints.h \
    CDefine.h \
    jtarget.h \
    jviewport.h \
    jgraphictarget.h \
    tinyxml/tinyxml2.h \
    c_config.h \
    c_radar_data.h

FORMS    += mainwindow.ui
#win32:CONFIG(release, debug|release): LIBS += -L$$PWD/shapelib/ -lshapelib
#else:unix: LIBS += -L$$PWD\shapelib\ -lshapelib
#INCLUDEPATH += $$PWD/shapelib
#DEPENDPATH += $$PWD/shapelib

INCLUDEPATH += $$PWD/WpdPack/Include
DEPENDPATH += $$PWD/WpdPack/Include
INCLUDEPATH += $$PWD/tinyxml/
DEPENDPATH += $$PWD/tinyxml/
INCLUDEPATH += $$PWD/
DEPENDPATH += $$PWD/
#RESOURCES +=
#INCLUDEPATH += $$PWD/../armadillo/include
#DEPENDPATH += $$PWD/../armadillo/include
#win32:LIBS += -L$$PWD/../armadilloWin32/lib_winx86/ -lblas_win32_MT
#win32:LIBS += -L$$PWD/../armadilloWin32/lib_winx86/ -llapack_win32_MT
#win32:LIBS += -L$$PWD/../armadillo/lib_winx86/ -lcbia.lib.blas.dyn.rel.x86.12
#win32:LIBS += -L$$PWD/../armadillo/lib_winx86/ -lcbia.lib.lapack.dyn.rel.x86.12

#unix: LIBS += -larmadillo
win32:LIBS += -L$$PWD//WpdPack/Lib/ -lPacket
win32:LIBS += -L$$PWD//WpdPack/Lib/ -lwpcap
#LIBS += -ltinyxml
#INCLUDEPATH += $$PWD/../opencv/build/include
#win32:LIBS += -L$$PWD/../opencv/build/x86/vc10/lib/ -lopencv_core249
#win32:LIBS += -L$$PWD/../opencv/build/x86/vc10/lib/ -lopencv_highgui249
#win32:LIBS += -L$$PWD/../opencv/build/x86/vc10/lib/ -lopencv_imgproc249
#win32:LIBS += -L$$PWD/../opencv/build/x86/vc10/lib/ -lopencv_videoio249

#LIBS +=

#win32:CONFIG(release, debug|release):INCLUDEPATH += $$PWD/armadillo/lib_winx86
#win32:CONFIG(release, debug|release):DEPENDPATH += $$PWD/armadillo/lib_winx86

#unix:QMAKE_CXXFLAGS += -O2
#unix:QMAKE_CXXFLAGS += -l -DARMA_DONT_USE_WRAPPER -lblas -llapack
DISTFILES += \
    appIcon.rc
win32:RC_FILE += appIcon.rc

