/*******************************************************************************
** Source File: ControlChannel.cpp -- Control channel of Client comm audio
**
** Program: Comm Audio
**
** Functions:
**      int setupControlChannel(hostent *);
**      DWORD WINAPI read(LPVOID);
**      void CALLBACK ReadRoutine(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
**      bool downloadSong(std::string);
**      bool requestSong(std::string);
**      int restartMulticast();
**      void handleEnd(std::string);
**      void handleControlMessage(ctrlMessage *);
**      void createControlString(ctrlMessage, std::string &);
**      void parseControlString(std::string, ctrlMessage *);
**      void updateListeners(std::vector<std::string>);
**      void updateNowPlaying(std::vector<std::string>);
**      void establishGUIConnector(void *);
**      void updateLibrary(std::vector<std::string>);
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
**  The Control Channel for Comm Audio. handles all control messages.
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
#include "socketinfo.h"
#include "controlChannel.h"
#include "unicastSong.h"
#include "mainwindow.h"
#include "multicast.h"
#include "tcpdownload.h"



using namespace std;

//hardcoded default control port
int port = 8000;
hostent *host;

//reading thread
HANDLE readThread = INVALID_HANDLE_VALUE;
HANDLE unicastThread = INVALID_HANDLE_VALUE;
HANDLE downloadThread = INVALID_HANDLE_VALUE;

int controlSocket;

//GUI Connector
MainWindow *GUI;
bool GUIReady;

SOCKET_INFORMATION controlSocketInfo;

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
** Function: read
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
    ZeroMemory(&(controlSocketInfo.overlapped), sizeof(WSAOVERLAPPED));
    controlSocketInfo.DataBuf.len = DATA_BUFSIZE;
    controlSocketInfo.DataBuf.buf = controlSocketInfo.Buffer;

    while(true)
    {
        if (WSARecv(controlSocket, &(controlSocketInfo.DataBuf), 1, &bytesReceived, &flags, &(controlSocketInfo.overlapped), ReadRoutine) == SOCKET_ERROR)
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
**              DWORD Error -- Errors occured during the routine
**              DWORD bytesReceived -- Number of bytes received
**              LPWSAOVERLAPPED -- Overlapped structure
**              DWORD inFlags -- Flags
**
**
** Returns:
**			void
**
** Notes:
** read routine of the control channel
**
*******************************************************************/
void CALLBACK ReadRoutine(DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD inFlags)
{

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
        //end the music
        closeAudio();
        EndMulticast();
        endUnicast();

        //open popup
        string message = "Server Closed";
        std::wstring stemp = std::wstring(message.begin(), message.end());
        LPCWSTR sw = stemp.c_str();
        MessageBox(NULL, sw, sw, MB_OK);

        //cleanup, close program
        closesocket(controlSocket);
        exit(0);
        return;
    }

    cout << "Received: " << sockInfo->Buffer;

    string received = sockInfo->Buffer;

    //parse control string into the structure
    parseControlString(received, &message);

    //handle it
    handleControlMessage(&message);
    cout.flush();

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

        //list of songs the server has
        case LIBRARY_INFO:
        {
            updateLibrary(cMessage->msgData);
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
            handleEnd(cMessage->msgData[0]);
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
    //wait for the GUI to be ready
    while(!GUIReady)
    {

    }

    //clear the list of listeners
    GUI->clearListeners();

    //add each listener to the GUI
    for (int i = 0; i < data.size(); i++)
    {
        GUI->updateListeners(data[i]);
    }
}

/*******************************************************************
** Function: updateLibrary
**
** Date: April 2nd, 2015
**
** Revisions:
**
**
** Designer: Rhea Lauzon
**
** Programmer: Rhea Lauzon
**
** Interface:
**			void updateLibrary(vector<string> data)
**              vector<string> data -- songs to be added
**
** Returns:
**			void
**
** Notes:
** Updates the library of music
*******************************************************************/
void updateLibrary(vector<string> data)
{
    //wait for the GUI to be ready
    while(!GUIReady)
    {

    }

    //add each song to the GUI
    for (int i = 0; i < data.size(); i++)
    {
        GUI->addSongToLibrary(data[i]);
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
    //wait for the GUI to be ready
    while(!GUIReady)
    {

    }

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
    GUIReady = true;
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
** Programmer: Rhea Lauzon, Jeff Bayntun
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
    //create a song request message for the control channel
    ctrlMessage message;
    string* to_send = new string(song);

    message.type = SAVE_SONG;
    message.msgData.emplace_back(song);

    string requestMessage;
    createControlString(message, requestMessage);

    //create the download thread
    HANDLE threadH;

    createWorkerThread(doTCPDownload, &threadH, to_send, 0);
        cout << "download worker created in main thread" << endl;
        fflush(stdout);
  //  Sleep(5000);
    //send the message to the server
    if (send(controlSocket, requestMessage.c_str(), requestMessage.length(), 0) == -1 )
    {
        cerr << "Send Failed" << endl;
        return false;
    }



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

    closeAudio();
    EndMulticast();

    //hide the pause/play button
    GUI->disableMulticastButton(true);
    //create the unicast thread
    DWORD threadId;
    if ((unicastThread = CreateThread(NULL, 0, unicastSong, (LPVOID) host, 0, &threadId)) == NULL)
    {
        cerr << "Unable to create unicast thread";
        return false;
    }

    return true;
}


/*******************************************************************
** Function: handleEnd
**
** Date: April 7th, 2015
**
** Revisions:
**
**
** Designer: Rhea Lauzon
**
** Programmer: Rhea Lauzon
**
** Interface:
**			void handleEnd(string type)
**              string type -- Type of end song message
**
** Returns:
**			void
**
** Notes:
** Handles the various end song messages
**********************************************************************/
void handleEnd(string type)
{
    //download song has completed
    if (type.compare("0") == 0)
    {
        string message = "Download Complete!";
        std::wstring stemp = std::wstring(message.begin(), message.end());
        LPCWSTR sw = stemp.c_str();
        MessageBox(NULL, sw, sw, MB_OK);

        return;
    }

    //unicast song has completed
    if(type.compare("1") == 0)
    {
        //end unicast
        endUnicast();

        //close all the audio
        closeAudio();

        //unicast has ended
        restartMulticast();

        GUI->disableMulticastButton(false);

        return;
    }

    //multicast song is swapping
    if (type.compare("2") == 0)
    {
        cout << "Swapping multicast" << endl;
        waitForNewSong();

        return;
    }
}



/*******************************************************************
** Function: restartMulticast
**
** Date: April 7th, 2015
**
** Revisions:
**
**
** Designer: Rhea Lauzon
**
** Programmer: Rhea Lauzon
**
** Interface:
**			int restartMulticast()
**
**
** Returns:
**			int -- -1 on failure, 0 on success
**
** Notes:
** Restarts the multicast server
**********************************************************************/
int restartMulticast()
{
    struct in_addr ia;
    memcpy((void*)host->h_addr,(void*)&ia, host->h_length);

    if (!StartMulticast(ia))
    {
        cerr << "Unable to restart multicast." << endl;
        return -1;
    }

    return 0;
}
