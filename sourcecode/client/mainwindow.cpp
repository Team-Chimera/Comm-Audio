
#define WIN32_LEAN_AND_MEAN
#include <QtWidgets>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dialog.h"
#include "multicast.h"

using std::string;
using std::vector;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    //set up the GUI
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::setupConnections()
{
    //add the connect button function push
    //QAction *connectButton = ui->menuBar->addAction("Connect");
    //connect(connectButton, SIGNAL(triggered()), this, SLOT(initialConnect()));


    //make now playing fields read only
    ui->artistName->setFocusPolicy(Qt::NoFocus);
    ui->artistName->setReadOnly(true);

    ui->songName->setFocusPolicy(Qt::NoFocus);
    ui->songName->setReadOnly(true);

    //create the dialog box for the launch
     Dialog initializeMessage(this);
     initializeMessage.exec();

}


void MainWindow::clearListeners()
{
    ui->listeners->clear();
}

void MainWindow::updateListeners(string listener)
{
    ui->listeners->addItem(QString::fromStdString(listener));
}

void MainWindow::updateNowPlaying(vector<string> songInfo)
{
    ui->songName->append(QString::fromStdString(songInfo[0]));
    ui->artistName->append(QString::fromStdString(songInfo[1]));
}
