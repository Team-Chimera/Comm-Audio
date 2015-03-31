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
#include "controlChannel.h"
#include "unicastSong.h"
#include "mainwindow.h"


using namespace std;

//hardcoded default control port
int port = 8000;
hostent *host;

//reading thread
HANDLE readThread = INVALID_HANDLE_VALUE;
HANDLE unicastThread = INVALID_HANDLE_VALUE;

int controlSocket;

//GUI Connector
MainWindow *GUI;

SOCKET_INFORMATION socketInfo;

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
**			int createControlChannel(hostent *hp)
**              hostent *hp -- IP of the server
**
** Returns:
**			int -- -1 on socket failure, 0 on successful
**                  client connection
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
**			DWORD WINAPI read(LPVOID arg)
**              LPVOID arg -- Thread argument
**
** Returns:
**			DWORD -- -1 on fail; 0 on success end of thread
**
** Notes:
** The read thread of the control thread. Reads data on the
** control channel.
*******************************************************************/
DWORD WINAPI read(LPVOID arg)
{
    DWORD bytesReceived = 0;
    int index;
    DWORD flags = 0;
    WSAEVENT eventArray[1];

    //set the socket structure
    //socketInfo.overlapped = {};
    ZeroMemory(&(socketInfo.overlapped), sizeof(WSAOVERLAPPED));
    socketInfo.DataBuf.len = DATA_BUFSIZE;
    socketInfo.DataBuf.buf = socketInfo.Buffer;

    while(true)
    {
        if (WSARecv(controlSocket, &(socketInfo.DataBuf), 1, &bytesReceived, &flags, &(socketInfo.overlapped), ReadRoutine) == SOCKET_ERROR)
        {
            if (WSAGetLastError() != WSA_IO_PENDING)
            {
                cerr << "WSARecv() failed. Error " << WSAGetLastError() << endl;
                return -1;
            }
        }

        eventArray[0] = WSACreateEvent();

        while(TRUE)
        {
          index = WSAWaitForMultipleEvents(1, eventArray, FALSE, WSA_INFINITE, TRUE);

          if (index == WAIT_IO_COMPLETION)
          {
              break;
          }
          else
          {
             cerr << "WSAWaitForMultipleEvents failed" << WSAGetLastError() << endl;
              return -1;
          }
        }
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
void CALLBACK ReadRoutine(DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD inFlags)
{
    DWORD recvBytes;
    DWORD flags;

    ctrlMessage message;
    SOCKET_INFORMATION * sockInfo = (SOCKET_INFORMATION *) Overlapped;

    if (error != 0)
    {
        cerr << "I/O Operation failed" << endl;
    }

    if (bytesTransferred == 0)
    {
        cerr << "Closing socket." << endl;
    }

    if (error != 0 || bytesTransferred == 0)
    {
        closesocket(controlSocket);
        return;
    }

    parseControlString(sockInfo->Buffer, &message);
    handleControlMessage(&message);

    cout << "Received: " << sockInfo->Buffer;
    cout.flush();
    Sleep(1000);


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
void createControlString(ctrlMessage msg, string &str)
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

/*******************************************************************
** Function: establishGUIConnector
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
**			void establishGUIConnector(void *gui)
**              void *gui -- Pointer to the main GUI
**
**
** Returns:
**			void
**
** Notes:
** Grabs a pointer to the main GUI for the control channel to manipulate
**********************************************************************/
void establishGUIConnector(void *gui)
{
    GUI = (MainWindow *)gui;
}


/*******************************************************************
** Function: downloadSong
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
**			bool downloadSong(string song)
**              string song -- Song that is being requested
**
**
** Returns:
**			void
**
** Notes:
** requests a song for downloading from the server
**********************************************************************/
bool downloadSong(string song)
{
    return true;
}


/*******************************************************************
** Function: requestSong
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
**			bool requestSong(string song)
**              string song -- Song that is being requested
**
**
** Returns:
**			bool -- True on success, false on failure
**
** Notes:
** requests a song for streaming unicastly from the server
**********************************************************************/
bool requestSong(string song)
{
    //create a song request message for the control channel
    ctrlMessage message;

    message.type = SONG_REQUEST;
    message.msgData.emplace_back(song);

    string requestMessage;
    createControlString(message, requestMessage);

    //send the message to the server
    if (send(controlSocket, requestMessage.c_str(), requestMessage.length(), 0) == -1 )
    {
        cerr << "Send Failed" << endl;
        return false;
    }

    //create the unicast thread
    DWORD threadId;
    if ((unicastThread = CreateThread(NULL, 0, unicastSong, (LPVOID) host, 0, &threadId)) == NULL)
    {
        cerr << "Unable to create unicast thread";
        return false;
    }


    return true;
}

