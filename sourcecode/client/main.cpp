#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <WS2tcpip.h>
#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    MainWindow GUI;

    //initialize the socket information
    int nRet;
    WSADATA stWSAData;
    nRet = WSAStartup(0x0202, &stWSAData);
     if (nRet)
     {
         printf ("WSAStartup failed: %d\r\n", nRet);
         exit(1);
     }

    GUI.setupConnections();
    GUI.show();

    return a.exec();
}
