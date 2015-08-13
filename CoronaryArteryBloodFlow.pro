#-------------------------------------------------
#
# Project created by QtCreator 2015-08-04T13:09:28
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CoronaryArteryBloodFlow
TEMPLATE = app


SOURCES  += main.cpp\
            viewerwidget.cpp \
            meshprocessor.cpp

HEADERS  += viewerwidget.h \
            meshprocessor.h

FORMS    += viewerwidget.ui

INCLUDEPATH  += "C:/Users/gobeawanl/Documents/MeshProcToolkit/Header"

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../ -lMeshProcToolKit
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../ -lMeshProcToolKitd

#INCLUDEPATH += $$PWD/../../
#DEPENDPATH += $$PWD/../../

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../../libMeshProcToolKit.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../../libMeshProcToolKitd.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../../MeshProcToolKit.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../../MeshProcToolKitd.lib
