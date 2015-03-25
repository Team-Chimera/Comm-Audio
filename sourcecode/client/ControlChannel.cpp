/*******************************************************************************
** Source File: ControlChannel.cpp -- Control channel of Client comm audio
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
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sstream>
#include <string>
#include <iomanip>
#include <list>
#include <iostream>
#include <vector>
#include <WinSock2.h>
#include "ControlChannel.h"
#include "mainwindow.h"


using namespace std;

//hardcoded default control port
int port = 8000;
hostent *host;

//reading thread
HANDLE readThread = INVALID_HANDLE_VALUE;

int controlSocket;

//GUI Connector
MainWindow *GUI;


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
int setupControlChannel(hostent *hp)
{
    sockaddr_in server;
    SOCKET control;
    server.sin_family = AF_INET;
    server.sin_port = htons(CONTROL_PORT);

    //copy the server address from the resolved host
    memcpy((char *) &server.sin_addr, hp->h_addr, hp->h_length);


    //create the control socket
    if ((control = socket( AF_INET, SOCK_STREAM, 0 ) ) == INVALID_SOCKET)
    {
        cout << "Cannot create control Socket.", "Error";
        return -1;
    }

    // Connecting to the server
    if (connect(control, (sockaddr *)&server, sizeof(server)) == -1)
    {
        cout << "Cannot connect to the server. Please try again.", "Error";
        return -1;
    }

    //add control socket & host info to the global
    controlSocket = control;
    host = hp;

    //begin receiving data as completion routine
    DWORD threadId;
    if ((readThread = CreateThread(NULL, 0, read, NULL, 0, &threadId)) == NULL)
    {
        cerr << "Unable to create accept thread";
        return -1;
    }
    return 0;
}

DWORD WINAPI read(LPVOID arg)
{
    //make the overlapped structure
    WSAOVERLAPPED Overlapped;
    WSABUF DataBuf;
    DWORD RecvBytes;
    DWORD Flags = 0;

    ZeroMemory(&Overlapped, sizeof(WSAOVERLAPPED));

    while (true)
    {
        //read from the server
        if(WSARecv(controlSocket, &DataBuf, 1, &RecvBytes, &Flags, &Overlapped, ReadRoutine) == SOCKET_ERROR)
        {
            if (WSAGetLastError() != WSA_IO_PENDING)
            {
                cerr << "WSARecv() failed";
                return -1;
            }
        }
        SleepEx(INFINITE, TRUE);
    }

    return 0;
}


/*******************************************************************
** Function: readRoutine
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
**			void CALLBACK ReadRoutine(DWORD Error,
**                  DWORD bytesReceived, LPWSAOVERLAPPED Overlapped, DWORD inFlags)
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
void CALLBACK ReadRoutine(DWORD Error, DWORD bytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD inFlags)
{
    DWORD SendBytes, RecvBytes;
    DWORD Flags;

    LPSOCKET_INFORMATION SI = (LPSOCKET_INFORMATION) Overlapped;

    if (Error != 0 || bytesTransferred == 0)
    {
        closesocket(controlSocket);
        return;
    }

    // Check to see if the BytesRECV field equals zero. If this is so, then
    // this means a WSARecv call just completed so update the BytesRECV field
    // with the BytesTransferred value from the completed WSARecv() call.
    if ( SI->BytesRECV == 0)
    {
        SI->BytesRECV = bytesTransferred;
        SI->BytesSEND = 0;

        ctrlMessage received;
        string receivedMessage;

        receivedMessage.assign(SI->Buffer);
        parseControlString(receivedMessage, &received);
        handleControlMessage(&received);

        GUI->updateListeners(receivedMessage);
    }
      SI->BytesRECV = 0;
      SI->bytesToSend = 0; // just in case it got negative somehow.....

      // Now that there are no more bytes to send post another WSARecv() request.

      Flags = 0;
      ZeroMemory(&(SI->Overlapped), sizeof(WSAOVERLAPPED));
      ZeroMemory(&(SI->Buffer), sizeof(DATA_BUFSIZE));

      SI->DataBuf.len = DATA_BUFSIZE;
      SI->DataBuf.buf = SI->Buffer; // should this be zeroed??

      if (WSARecv(SI->Socket, &(SI->DataBuf), 1, &RecvBytes, &Flags,
         &(SI->Overlapped), ReadRoutine) == SOCKET_ERROR)
      {
         if (WSAGetLastError() != WSA_IO_PENDING )
         {
            printf("WSARecv() failed with error %d\n", WSAGetLastError());
            return;
         }
      }

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
    msg->type = static_cast<messageType> (atoi(str.substr(0, type).c_str()));

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
            updateNowPlaying(cMessage->msgData);
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
            updateListeners(cMessage->msgData);
            break;
        }
    }
}


/*******************************************************************
** Function: updateListeners
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
**			void updateListeners(vector<string> data)
**              vector<string> data --clients to be added
**
**
** Returns:
**			void
**
** Notes:
** Updates the current listeners.
*******************************************************************/
void updateListeners(vector<string> data)
{
    //clear the list of listeners
    GUI->clearListeners();

    //add each listener to the GUI
    for (int i = 0; i < data.size(); i++)
    {
        GUI->updateListeners(data[i]);
    }
}


/*******************************************************************
** Function: updateNowPlaying
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
**			void updateNowPlaying(vector<string> msgData)
**              vector<string> msgData -- Song data
**
**
** Returns:
**			void
**
** Notes:
** Updates the now playing tab with the information from the server.
*******************************************************************/
void updateNowPlaying(vector<string> msgData)
{
    //fetch the song
    string nowPlaying = msgData[0];
    vector<string> songData;

    //begin parsing out the song information into a vector
    while(nowPlaying.length() > 0)
    {
        int endSection = nowPlaying.find('^');
        songData.push_back(nowPlaying.substr(0, endSection));

        //chop off that section
        nowPlaying = nowPlaying.substr(endSection + 1, nowPlaying.length());
    }

    GUI->updateNowPlaying(songData);

}

void establishGUIConnector(void *gui)
{
    GUI = (MainWindow *)gui;
}
