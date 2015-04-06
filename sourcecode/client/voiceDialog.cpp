#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <string>
#include <QMessageBox>
#include "microphone.h"
#include "voiceDialog.h"
#include "ui_voicedialog.h"
#include "player.h"

using std::string;
using std::cerr;

HANDLE voiceThread = INVALID_HANDLE_VALUE;


Microphone *voice;

/*****************************************************************
** Function: voiceDialog
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
**			voiceDialog(QWidget *parent)
**              QWidget *parent -- parent widget
**
** Returns:
**          N/A (Constructor)
**
** Notes:
** client context menu constructor
**
*******************************************************************/
voiceDialog::voiceDialog(Microphone *mic, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::voiceDialog)
{
    ui->setupUi(this);

    voice = mic;

    //add the connection to the two buttons
    connect(ui->connectVoice, SIGNAL(pressed()), this, SLOT(startVoiceChat()));
}


/*****************************************************************
** Function: ~voiceDialog (Destructor)
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
**			~voiceDialog()
**
** Returns:
**          N/A (Destructor)
**
** Notes:
** Destructs the unicast dialog object
**
*******************************************************************/
voiceDialog::~voiceDialog()
{
    delete ui;
}



/*****************************************************************
** Function: setClientName
**
** Date: April 4th, 2015
**
** Revisions:
**
**
** Designer: Rhea Lauzon
**
** Programmer: Rhea Lauzon
**
** Interface:
**			void setClientName(string client)
**          string client -- Name of the client that was selected
**
** Returns:
**          void
**
** Notes:
** Sets the specified client name to the one clicked on.
*******************************************************************/
void voiceDialog::setClientName(string client)
{
    clickedClient = client;
    ui->client->setText(QString::fromStdString(client));
}



/*****************************************************************
** Function: triggerVoiceChat
**
** Date: April 4th, 2015
**
** Revisions:
**
**
** Designer: Rhea Lauzon
**
** Programmer: Rhea Lauzon
**
** Interface:
**			bool startVoiceChat()
**
**
** Returns:
**          bool -- if the connect fails or starts
**
** Notes:
** Toggles between using the microphone or not
*******************************************************************/
bool voiceDialog::triggerVoiceChat()
{
    DWORD threadId;

    if(!connected)
    {
        connected = true;
        voice->startVoice(QString::fromStdString(clickedClient));
        QPixmap pixmap(":/images/mic.png");
        QIcon ButtonIcon(pixmap);
        ui->connectVoice->setIcon(ButtonIcon);
        ui->connectVoice->setIconSize(pixmap.rect().size());

    }
    else
    {
        connected = false;
        voice->stopVoice();
    }

    return true;
}


/*****************************************************************
** Function: reject
**
** Date: March 18th, 2015
**
** Revisions:
**
**
** Designer: Rhea Lauzon
**
** Programmer: Rhea Lauzon
**
** Interface:
**          void reject()
**
** Returns:
**          void
**
** Notes:
** Overriden method to trigger close events when disconnecting.
**
*******************************************************************/
void voiceDialog::reject()
{

    QMessageBox::StandardButton resBtn = QMessageBox::Yes;

        resBtn = QMessageBox::question( this, "End Voice Chat",
                                        tr("Are you sure you want to quit?\n"),
                                        QMessageBox::Cancel | QMessageBox::Yes,
                                        QMessageBox::Yes);
    if (resBtn == QMessageBox::Yes)
    {
        QDialog::reject();

        //End the voice
        connected = false;
        voice->stopVoice();
    }
}
