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


SOURCES += main.cpp \
        mainwindow.cpp \
        multicast.cpp \
        voiceChat.cpp \
        controlChannel.cpp \
        dialog.cpp \
        unicastSong.cpp \
    unicastdialog.cpp \
    downloadSong.cpp \
    tcpdownload.cpp

HEADERS  += mainwindow.h \
    client.h \
    multicast.h \
    voiceChat.h \
    controlMessage.h \
    unicastSong.h \
    dialog.h \
    unicastdialog.h \
    controlChannel.h \
    music.h \
    socketinfo.h \
    downloadSong.h \
    tcpdownload.h

FORMS    += mainwindow.ui \
    dialog.ui \
    unicastdialog.ui


INSTALLS += install_it

LIBS += -lws2_32 -lwinmm
CONFIG += c++11
