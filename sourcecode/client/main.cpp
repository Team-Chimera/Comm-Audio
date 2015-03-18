#include "mainwindow.h"
#include <QApplication>
#include "controlMessage.h"
#include "ControlChannel.h"
int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    MainWindow w;
    w.setupConnections();
    w.show();

    hostent *hp;
    setupControlChannel(5000, hp);

    return a.exec();
}
