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

    return a.exec();
}
