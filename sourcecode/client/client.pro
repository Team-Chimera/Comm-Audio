#-------------------------------------------------
#
# Project created by QtCreator 2015-02-27T19:05:58
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

install_it.path = %{buildDir}


TARGET = client
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    multicast.cpp \
    voiceChat.cpp \
    ControlChannel.cpp \
    unicastSong.cpp

HEADERS  += mainwindow.h \
    client.h \
    multicast.h \
    voiceChat.h \
    ControlChannel.h \
    controlMessage.h \
    unicastSong.h

FORMS    += mainwindow.ui


INSTALLS += install_it
