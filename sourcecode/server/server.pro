#-------------------------------------------------
#
# Project created by QtCreator 2015-03-02T19:15:56
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

install_it.path = %{buildDir}


TARGET = server
TEMPLATE = app


SOURCES += main.cpp\
mainwindow.cpp \
network.cpp \
session.cpp \
helper.cpp

HEADERS += mainwindow.h \
network.h \
session.h \
helper.h

FORMS    += mainwindow.ui


INSTALLS += install_it

LIBS += -lws2_32
CONFIG += c++11
