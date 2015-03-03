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
   //add the open file function the the load songs button
   QAction *loadSongs = ui->menuBar->addAction("Load Songs");
   connect(loadSongs, SIGNAL(triggered()), this, SLOT(openFile()));

   //add the connect button function push
   QAction *connectButton = ui->menuBar->addAction("Connect");
   connect(connectButton, SIGNAL(triggered()), this, SLOT(initialConnect()));
}


void MainWindow::openFile()
{
    const QStringList musicPaths = QStandardPaths::standardLocations(QStandardPaths::MusicLocation);
    const QStringList fileList =
        QFileDialog::getOpenFileNames(this, tr("Open File"),
                                     musicPaths.isEmpty() ? QDir::homePath() : musicPaths.first(),
                                     tr("WMA Files (*.wma);;All files (*.*)"));

    //add the file names to the list if files were selected
    if (fileList.size() > 0)
    {
        for (int i = 0; i < fileList.size(); i++)
        {
            //fetch the file name
            QString filePathName = QFileInfo(fileList[i]).fileName();

            //add the song to the list
            ui->songs->addItem(filePathName);
        }
    }
}

bool MainWindow::initialConnect()
{
    //do later, i assume will call a socket connect function
    // to get stuff out of the GUI land.
    //Please don't write socket stuff in the GUI!!!

    return true;
}
