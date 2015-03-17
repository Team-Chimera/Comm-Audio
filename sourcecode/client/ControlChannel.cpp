/*******************************************************************************
** Source File: ControlChannel.cpp -- Control channel of Client comm audio
**
** Program: Protocol Analysis
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
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#include <winsock2.h>

#include "controlMessage.h"
#include "ControlChannel.h"
#include "unicastSong.h"

using std::stringstream;
using std::string;
using std::vector;
using std::ofstream;

//hardcoded default control port
int port = 8000;
hostent *host;

int controlSocket;



/*******************************************************************
** Function: setupControlChannel
**
** Date: March 14th, 2015
**
** Revisions:
**
**
** Designer: Rhea Lauzon
**
** Programmer: Rhea Lauzon
**
** Interface:
**			int createControlChannel(int port, hostent *hp)
**              int port -- Control channel port
**              hostent *hp -- IP of the server
**
** Returns:
**			int
**
** Notes:
** Called to create the control channel connect on the client
**
*******************************************************************/
int setupControlChannel(int port, hostent *hp)
{
    sockaddr_in server;
    SOCKET control;
    server.sin_family = AF_INET;
    server.sin_port = htons (port);

    //copy the server address from the resolved host
    memcpy((char *) &server.sin_addr, hp->h_addr, hp->h_length);

    //create the control socket
    if ( ( control = socket( AF_INET, SOCK_STREAM, 0 ) ) == INVALID_SOCKET )
    {
        cout << "Cannot create control Socket.", "Error";
        return -1;
    }
    // Connecting to the server
    if ( connect( control, ( sockaddr * )&server, sizeof( server ) ) == -1 )
    {
        cout << "Cannot connect to the server. Please try again.", "Error";
        return -1;
    }

    //add control socket & host info to the struct
    controlSocket = control;
    host = hp;

}

/*******************************************************************
** Function: createControlString
**
** Date: March 14th, 2015
**
** Revisions:
**
**
** Designer: Rhea Lauzon
**
** Programmer: Rhea Lauzon
**
** Interface:
**			void createControlString(ctrlMessage &msg, string &str)
**              ctrlMessage &msg -- Message to create from
**              string &str -- Reference of string to place data in
**
**
** Returns:
**			void
**
** Notes:
** Converts a control string structure to a string to send
*******************************************************************/
void createControlString(ctrlMessage &msg, string &str)
{
    //add the type to the string
    stringstream ss;
    ss << msg.type << "~";

    //loop through all the data in the array
    for (int i = 0; i < (int) msg.msgData.size(); i++)
    {
        ss << msg.msgData[i] << "|";
    }

    str = ss.str();

}

/*******************************************************************
** Function: parseControlString
**
** Date: March 14th, 2015
**
** Revisions:
**
**
** Designer: Rhea Lauzon
**
** Programmer: Rhea Lauzon
**
** Interface:
**			void parseControlString(string str, ctrlMessage *msg)
**              string str -- string received
**              ctrlMessage *msg - The control message received
**
** Returns:
**			int
**
** Notes:
** Handles all control messages received by the control channel
*******************************************************************/
void parseControlString(string str, ctrlMessage *msg)
{
    //get the type of the message
    int type = str.find('~');
    msg->type = static_cast<MessageType> (atoi(str.substr(0, type).c_str()));

    //chunk off the message's type
    str = str.substr(type +1, str.length());

    //parse the message's data out of the remaining string
    while(str.length() > 0)
    {
        int endData = str.find('|');
        msg->msgData.emplace_back(str.substr(0, endData));

        str = str.substr(endData + 1, str.length());
    }

}

/*******************************************************************
** Function: handleControlMessage
**
** Date: March 14th, 2015
**
** Revisions:
**
**
** Designer: Rhea Lauzon
**
** Programmer: Rhea Lauzon
**
** Interface:
**			void handleControlMessage(ctrlMessage *cMessage)
**              ctrlMessage *cMessage - The control message received
**
** Returns:
**			int
**
** Notes:
** Handles all control messages received by the control channel
*******************************************************************/
void handleControlMessage(ctrlMessage *cMessage)
{
    //switch on the message's type
    switch(cMessage->type)
    {
       //end connection received from server
        case END_CONNECTION:
        {
            break;
        }

        //someone wants to make a mic chat with this client
        case MIC_CONNECTION:
        {
            break;
        }

        //list of songs the server has
        case LIBRARY_INFO:
        {
            break;
        }

        //new song is playing
        case NOW_PLAYING:
        {
            break;
        }

        //song is ending
        case END_SONG:
        {
            break;
        }

        //liste of current listeners
        case CURRENT_LISTENERS:
        {
            break;
        }
    }
}
