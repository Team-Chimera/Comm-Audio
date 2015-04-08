/**********************************************************************
 *
**	SOURCE FILE:	dialog.cpp -  Custom dialog class
**
**	PROGRAM:	Comm Audio
**
**	FUNCTIONS:
**          explicit Dialog(QWidget *parent = 0);
**          ~Dialog();
**          void reject();
**          bool initialConnect(QString);
**          bool connectMulticast();
**
**
**	DATE: 		March 7th, 2015
**
**
**	DESIGNER:	Rhea Lauzon A00881688
**
**
**	PROGRAMMER: Rhea Lauzon A00881688
**
**	NOTES:
**	Dialog class for the launcher
*************************************************************************/

#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <QMessageBox>
#include "dialog.h"
#include "ui_dialog.h"
#include "controlChannel.h"
#include "unicastSong.h"
#include "multicast.h"
#include "downloadSong.h"

using namespace std;


/*****************************************************************
** Function: Dialog
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
**			Dialog(QWidget *parent)
**              QWidget *parent -- parent widget
**
** Returns:
**          N/A (Constructor)
**
** Notes:
** Dialog window constructor.
**
*******************************************************************/
Dialog::Dialog(QWidget *parent) :QDialog(parent), ui(new Ui::Dialog)
{
    ui->setupUi(this);

    //add the connect button function on push
    connect(ui->connectButton, SIGNAL(pressed()), this, SLOT(connectMulticast()));
}


/*****************************************************************
** Function: ~Dialog (Destructor)
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
**			~Dialog()
**
** Returns:
**          N/A (Destructor)
**
** Notes:
** Destructs the dialog object
**
*******************************************************************/
Dialog::~Dialog()
{
    delete ui;
}


/*****************************************************************
** Function: connectMulticast
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
**			bool connectMulticast()
**
** Returns:
**          bool -- True upon successful connection,
**                  False on fail
**
** Notes:
** Calls the connect multicast function and opens the main GUI
*******************************************************************/
bool Dialog::connectMulticast()
{
    //get the user entered address
    QString address = ui->addressField->text();

    //check if there is data
    if (address.isEmpty())
    {
        return false;
    }
    else
    {
        //connected to the server succesfully
        if ( Dialog::initialConnect(address) )
        {
            this->hide();
        }
        else
        {
            cerr << "cannot connect :( connect to server failed.";
            return false;
        }
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
void Dialog::reject()
{

    QMessageBox::StandardButton resBtn = QMessageBox::Yes;

        resBtn = QMessageBox::question( this, "Close the Audio Streamer",
                                        tr("Are you sure you want to quit?\n"),
                                        QMessageBox::Cancel | QMessageBox::Yes,
                                        QMessageBox::Yes);
    if (resBtn == QMessageBox::Yes)
    {
        QDialog::reject();
        //close the entire program
        exit(0);
    }
}


/*******************************************************************
** Function: initialConnect
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
**			bool initialConnect(QString address)
**              QString address -- Address to conect to
**
**
** Returns:
**			bool -- returns true on success and false on fail
**
** Notes:
** Does the initial connection on program start.
**********************************************************************/
bool Dialog::initialConnect(QString address)
{
    struct hostent *he;
    struct hostent *multi;

    //convert the QString to a string
    string IP = address.toStdString();
    string multicastIP = "234.5.6.7";


     /* resolve hostname */
     if ((he = gethostbyname(IP.c_str())) == NULL)
     {
         //error getting the host
         cerr << "Failed to retrieve host" << endl;
         string message = "Error Retrieving Host";
         std::wstring stemp = std::wstring(message.begin(), message.end());
         LPCWSTR sw = stemp.c_str();
         MessageBox(NULL, sw, sw, MB_ICONERROR);
         return false;
     }

     //create the control channel

    if (setupControlChannel(he) < 0)
    {
        cerr << "Unable to open control channel." << endl;
        exit(1);
    }

     //Resolve multicast
     if ((multi = gethostbyname(multicastIP.c_str())) == NULL)
     {
         return false;
     }

     struct in_addr ia;
     memcpy((void*)multi->h_addr,(void*)&ia, multi->h_length);
     StartMulticast(ia);


    return true;
}
