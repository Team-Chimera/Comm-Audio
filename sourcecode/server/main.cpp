#include "mainwindow.h"
#include <QApplication>
#include <WinSock2.h>

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    MainWindow w;
    w.setupConnections();
    w.show();

    return a.exec();
}
