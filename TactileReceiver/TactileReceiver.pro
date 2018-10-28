#-------------------------------------------------
#
# Project created by QtCreator 2018-01-27T19:30:34
#
#-------------------------------------------------

QT       += core gui
QT       += charts
QT       += testlib

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = TactileReceiver
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
INCLUDEPATH += "eigen"
INCLUDEPATH += "gnuplot_i"
INCLUDEPATH += "EchoStateNetwork"

LIBS += -L/usr/lib/python3.5/config-3.5m-x86_64-linux-gnu
LIBS += -L/usr/lib -lpython3.5m
#INCLUDEPATH += /usr/include/python3.5
#DEPENDPATH += /usr/include/python3.5
# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    networkthread.cpp \
    experiment.cpp \
    echostatenetwork.cpp \
    qcustomplot.cpp \
    echostaterunnable.cpp

HEADERS += \
        mainwindow.h \
    networkthread.h \
    experiment.h \
    echostatenetwork.h \
    qcustomplot.h \
    echostaterunnable.h

FORMS += \
        mainwindow.ui

DISTFILES += \
    Button.qml
