#include <iostream>
#include <string>
#include "voiceDialog.h"
#include "ui_voicedialog.h"
#include "player.h"
#include "microphone.h"

using std::string;


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
voiceDialog::voiceDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::voiceDialog)
{
    ui->setupUi(this);

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
** Function: startVoiceChat
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
** Starts a voice chat session with the specified client
*******************************************************************/
bool voiceDialog::startVoiceChat()
{
    Microphone mic(QString::fromStdString(clickedClient));
    Player play;

    return true;
}
