#include "mainwindow.h"
#include <QApplication>
#include "controlChannel.h"

using std::cout;


int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    MainWindow w;
    w.setupConnections();
    w.show();

    //temporary do the WSA startup
    int ret;
    WSADATA wsaData;
    if ((ret = WSAStartup(0x0202,&wsaData)) != 0)
     {
        cout << "WSA Error";
        WSACleanup();
        return -1;
     }

    openControlListener();

    return a.exec();
}
