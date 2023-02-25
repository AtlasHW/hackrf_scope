#-------------------------------------------------
#
# Project created by QtCreator 2023-02-06T22:12:33
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = hackrf_scope
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    oscilloscope.cpp \
    encoder.cpp \
    preview.cpp \
    scopeview.cpp

HEADERS  += mainwindow.h \
    oscilloscope.h \
    encoder.h \
    preview.h \
    scopeview.h

FORMS    += mainwindow.ui \
    oscilloscope.ui



unix:!macx: LIBS += -L$$PWD/../../../../usr/local/lib/ -lhackrf

INCLUDEPATH += $$PWD/../../../../usr/local/include/libhackrf
DEPENDPATH += $$PWD/../../../../usr/local/include/libhackrf

unix:!macx: PRE_TARGETDEPS += $$PWD/../../../../usr/local/lib/libhackrf.a
