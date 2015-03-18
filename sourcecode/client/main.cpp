#include "mainwindow.h"
#include <QApplication>

#include "multicast.h"
int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    MainWindow w;
    w.setupConnections();
    w.show();

    return a.exec();
}
