#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    MainWindow GUI;
    GUI.setupConnections();
    GUI.show();

    return a.exec();
}
