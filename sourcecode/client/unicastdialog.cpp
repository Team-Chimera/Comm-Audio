#include <iostream>
#include <string>
#include "unicastdialog.h"
#include "ui_unicastdialog.h"
#include "controlChannel.h"
#include "tcpdownload.h"

using std::string;


/*****************************************************************
** Function: unicastDialog
**
** Date: March 27th, 2015
**
** Revisions:
**
**
** Designer: Rhea Lauzon
**
** Programmer: Rhea Lauzon
**
** Interface:
**			unicastDialog(QWidget *parent)
**              QWidget *parent -- parent widget
**
** Returns:
**          N/A (Constructor)
**
** Notes:
** Song context menu constructor
**
*******************************************************************/
unicastDialog::unicastDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::unicastDialog)
{
    ui->setupUi(this);

    //add the connection to the two buttons
    connect(ui->playUnicast, SIGNAL(pressed()), this, SLOT(unicastSong()));
    connect(ui->DownloadSong, SIGNAL(pressed()), this, SLOT(requestDownload()));

    //add the icons
    QPixmap pixmap(":/Images/images/download.png");
    QIcon ButtonIcon(pixmap);
    ui->DownloadSong->setIcon(ButtonIcon);
    ui->DownloadSong->setIconSize(QSize(ui->DownloadSong->size().width(),ui->DownloadSong->size().height()));

    QPixmap pixmap2(":/Images/images/play.png");
    QIcon ButtonIcon2(pixmap2);
    ui->playUnicast->setIcon(ButtonIcon2);
    ui->playUnicast->setIconSize(QSize(ui->playUnicast->size().width(),ui->playUnicast->size().height()));


}


/*****************************************************************
** Function: ~unicastDialog (Destructor)
**
** Date: March 27th, 2015
**
** Revisions:
**
**
** Designer: Rhea Lauzon
**
** Programmer: Rhea Lauzon
**
** Interface:
**			~unicastDialog()
**
** Returns:
**          N/A (Destructor)
**
** Notes:
** Destructs the unicast dialog object
**
*******************************************************************/
unicastDialog::~unicastDialog()
{
    delete ui;
}


/*****************************************************************
** Function: requestDownload
**
** Date: March 27th, 2015
**
** Revisions:
**
**
** Designer: Rhea Lauzon
**
** Programmer: Rhea Lauzon
**
** Interface:
**			void requestDownload()
**
** Returns:
**          void
**
** Notes:
** Sends a messsage to the server requesting a file download
*******************************************************************/
void unicastDialog::requestDownload()
{
  // get name of current song, send appropriate request to server
    downloadSong(clickedSong);

    // make thread to handle download
    // in thread, make rcv call, get alertable, then do that shit.
    this->close();
}

/*****************************************************************
** Function: unicastSong
**
** Date: March 27th, 2015
**
** Revisions:
**
**
** Designer: Rhea Lauzon
**
** Programmer: Rhea Lauzon
**
** Interface:
**			void unicastSong()
**
** Returns:
**          void
**
** Notes:
** Requests a song from the server to be played as unicast
*******************************************************************/
void unicastDialog::unicastSong()
{
    requestSong(clickedSong);
    this->close();
}

/*****************************************************************
** Function: setSongName
**
** Date: March 27th, 2015
**
** Revisions:
**
**
** Designer: Rhea Lauzon
**
** Programmer: Rhea Lauzon
**
** Interface:
**			void setSongName(string song)
**          string song -- Name of the song that was selected
**
** Returns:
**          void
**
** Notes:
** Sets the specified song name to the one clicked on.
*******************************************************************/
void unicastDialog::setSongName(string song)
{
    clickedSong = song;
    ui->song->setText(QString::fromStdString(song));
}
