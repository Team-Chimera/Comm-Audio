/*******************************************************************************
** Source File: mainWindow.cpp -- main GUI for comm audio
**
** Program: Comm Audio
**
** Functions:
**
**
** Date: March 14th, 2015
**
** Revisions: N/A
**
** Designer: Rhea Lauzon
**
** Programmer: Rhea Lauzon
**
** Notes:
**
**
*****************************************************************************/

#define WIN32_LEAN_AND_MEAN
#include <QtWidgets>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include "mainwindow.h"
#include "unicastdialog.h"
#include "ui_mainwindow.h"
#include "dialog.h"
#include "multicast.h"
#include "voiceDialog.h"
#include "controlChannel.h"
#include "player.h"
#include "microphone.h"

using std::string;
using std::vector;

Microphone *mic;
Player *micPlayer;

/*****************************************************************
** Function: MainWindow
**
** Date: March 7th, 2015
**
** Revisions:
**
**
** Designer: Rhea Lauzon
**
** Programmer: Rhea Lauzon
**
** Interface:
**			MainWindow(QWidget *parent)
**              QWidget *parent -- parent widget
**
** Returns:
**          N/A (Constructor)
**
** Notes:
** Main window's GUI constructor
**
*******************************************************************/
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

    //set up the GUI
    ui->setupUi(this);

    mic = new Microphone();
    micPlayer = new Player;



}


/*****************************************************************
** Function: ~MainWindow (Destructor)
**
** Date: March 7th, 2015
**
** Revisions:
**
**
** Designer: Rhea Lauzon
**
** Programmer: Rhea Lauzon
**
** Interface:
**			~MainWindow()
**
** Returns:
**          N/A (Destructor)
**
** Notes:
** Destructs the main window object
**
*******************************************************************/
MainWindow::~MainWindow()
{
    delete ui;
}


/*****************************************************************
** Function: setupConnections
**
** Date: March 7th, 2015
**
** Revisions:
**
**
** Designer: Rhea Lauzon
**
** Programmer: Rhea Lauzon
**
** Interface:
**			void setupConnections()
**
** Returns:
**          void
**
** Notes:
** Sets up all on-click listeners and GUI attributes.
*******************************************************************/
void MainWindow::setupConnections()
{
    //add the connect button function push
    //QAction *connectButton = ui->menuBar->addAction("Connect");
    //connect(connectButton, SIGNAL(triggered()), this, SLOT(initialConnect()));

    //make now playing fields read only
    ui->artistName->setFocusPolicy(Qt::NoFocus);
    //ui->artistName->setReadOnly(true);

    ui->songName->setFocusPolicy(Qt::NoFocus);
    //ui->songName->setReadOnly(true);

    //make the songs in the library clickable
    connect(ui->songs, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(openSongMenu(QListWidgetItem *)));

    //make the client list clickable
    connect(ui->listeners, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(openVoiceMenu(QListWidgetItem *)));


    //create the dialog box for the launch
     Dialog initializeMessage(this);
     initializeMessage.exec();


}


/*****************************************************************
** Function: clearListeners
**
** Date: March 7th, 2015
**
** Revisions:
**
**
** Designer: Rhea Lauzon
**
** Programmer: Rhea Lauzon
**
** Interface:
**			void clearListeners()
**
** Returns:
**          void
**
** Notes:
** Clears the list of listeners
*******************************************************************/
void MainWindow::clearListeners()
{
    ui->listeners->clear();
}


/*****************************************************************
** Function: updateListeners
**
** Date: March 7th, 2015
**
** Revisions:
**
**
** Designer: Rhea Lauzon
**
** Programmer: Rhea Lauzon
**
** Interface:
**			void updateListeners(string listener)
**              string listener -- listener to be added
**
** Returns:
**          void
**
** Notes:
** Adds a listener to the list of listeners
*******************************************************************/
void MainWindow::updateListeners(string listener)
{
    ui->listeners->addItem(QString::fromStdString(listener));
}


/*****************************************************************
** Function: updateNowPlaying
**
** Date: March 7th, 2015
**
** Revisions:
**
**
** Designer: Rhea Lauzon
**
** Programmer: Rhea Lauzon
**
** Interface:
**			void updateNowPlaying(vector<string> songInfo)
**              vector<string> song -- information on the current song
**
** Returns:
**          void
**
** Notes:
** Updates the now playing tab with current song information
*******************************************************************/
void MainWindow::updateNowPlaying(vector<string> songInfo)
{
    ui->songName->setText(QString::fromStdString(songInfo[0]));
    ui->artistName->setText(QString::fromStdString(songInfo[1]));
}


/*****************************************************************
** Function: addSongToLibrary
**
** Date: March 24th, 2015
**
** Revisions:
**
**
** Designer: Rhea Lauzon
**
** Programmer: Rhea Lauzon
**
** Interface:
**			void addSongToLibrary(string song)
**              string song --the song to be added
**
** Returns:
**          void
**
** Notes:
** Adds a song to the library list
*******************************************************************/
void MainWindow::addSongToLibrary(std::string s)

{
    ui->songs->addItem(QString::fromStdString(s));
}


/*****************************************************************
** Function: openSongMenu
**
** Date: March 24th, 2015
**
** Revisions:
**
**
** Designer: Rhea Lauzon
**
** Programmer: Rhea Lauzon
**
** Interface:
**			void openSongMenu(QListWidgetItem *it)
**              QListWidgetItem *it -- Item that was clicked on
**
** Returns:
**          void
**
** Notes:
** Opens a small context menu when each library item is clicked
*******************************************************************/
void MainWindow::openSongMenu(QListWidgetItem *it)
{
    //create the dialog box for the song
     unicastDialog songMenu (this);
     songMenu.setSongName(it->text().toStdString());
     songMenu.exec();
}


/*****************************************************************
** Function: openVoiceMenu
**
** Date: March 24th, 2015
**
** Revisions:
**
**
** Designer: Rhea Lauzon
**
** Programmer: Rhea Lauzon
**
** Interface:
**			void openVoiceMenu(QListWidgetItem *it)
**              QListWidgetItem *it -- Item that was clicked on
**
** Returns:
**          void
**
** Notes:
** Opens a small context menu when each client is opened
*******************************************************************/
void MainWindow::openVoiceMenu(QListWidgetItem *it)
{
    micPlayer->startVoicePlay();

    //create the dialog box for the song
     voiceDialog voiceMenu(mic, this);
     voiceMenu.setClientName(it->text().toStdString());
     voiceMenu.exec();

}

