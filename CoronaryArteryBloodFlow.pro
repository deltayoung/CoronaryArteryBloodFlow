#-------------------------------------------------
#
# Project created by QtCreator 2015-08-04T13:09:28
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CoronaryArteryBloodFlow
TEMPLATE = app


SOURCES += main.cpp\
        viewerwidget.cpp \
    meshprocessor.cpp

HEADERS  += viewerwidget.h \
    meshprocessor.h

FORMS    += viewerwidget.ui

INCLUDEPATH += "C:/Users/gobeawanl/Documents/MeshProcToolkit/Header"
