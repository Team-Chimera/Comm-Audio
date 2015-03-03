#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtWidgets>

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
    QAction *connectButton = ui->menuBar->addAction("Connect");
    connect(connectButton, SIGNAL(triggered()), this, SLOT(initialConnect()));
}


bool MainWindow::initialConnect()
{
    //do later, i assume will call a socket connect function
    // to get stuff out of the GUI land.
    //Please don't write socket stuff in the GUI!!!

    return true;
}

