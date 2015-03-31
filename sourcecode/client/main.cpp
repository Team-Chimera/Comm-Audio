#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <WS2tcpip.h>
#include "mainwindow.h"
#include "controlChannel.h"
#include <QApplication>


/*******************************************************************
** Function: main
**
** Date: March 16th, 2015
**
** Revisions:
**
**
** Designer: Rhea Lauzon
**
** Programmer: Rhea Lauzon
**
** Interface:
**			int main(int argc, char *argv[])
**              int argc -- number of command line arguments
**              char * argv[] -- Arguments
**
**
** Returns:
**			int
**
** Notes:
** Driver of the client program.
**********************************************************************/
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
    establishGUIConnector(&GUI);

    return a.exec();
}
