
#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <string>
#include <mmsystem.h>
#include "multicast.h"
#include "music.h"
#include "socketinfo.h"
#include "unicastSong.h"

using namespace std;

//UDP data socket
SOCKET unicastSongSocket;

HANDLE streamThread = INVALID_HANDLE_VALUE;

//flag for the song being finished
bool finished = false;


//server structure
sockaddr_in server;

//circular buffer
CircularBuffer * uniCircBuf;

/*******************************************************************
** Function: unicastSong
**
** Date: March 28th, 2015
**
** Revisions:
**
**
** Designer: Rhea Lauzon
**
** Programmer: Rhea Lauzon
**
** Interface:
**			DWORD WINAPI unicastSong(LPVOID host)
**              LPVOID host -- Host structure passed into the thread
**
** Returns:
**			DWORD -- -1 on failure of creating UDP socket,
**                      0 on success
**
** Notes:
** Creates the Unicast UDP socket and begins reading song data.
**
*******************************************************************/
DWORD WINAPI unicastSong(LPVOID host)
{
    uniCircBuf = getCircularBuffer();
    //initialize the buffer position to 0
    uniCircBuf->pos = 0;

    //conver tthe parameter
    hostent *hp = (hostent *) host;

    //set up the structure
    server.sin_family = AF_INET;
    server.sin_port = htons(CLIENT_UNICAST_PORT);
    server.sin_addr.s_addr = htonl(INADDR_ANY);


    // Copy the server address from the resolved host
    memcpy((char *)&server.sin_addr, hp->h_addr, hp->h_length);

    //create the UDP socket
    if ((unicastSongSocket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
    {
        cerr << "Unable to creat the Unicast UDP socket." << endl;
        return -1;
    }

    //bind the socket
    if (bind(unicastSongSocket, (struct sockaddr *) &server, sizeof(server)) == -1)
    {
        // The socket failed to be bound
        cerr << "Failed to bind unicast socket." << endl;
        WSACleanup();
        return -1;
    }


    //set to re-use address state
    BOOL flag = true;
    if (setsockopt(unicastSongSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&flag, sizeof(flag)) == SOCKET_ERROR)
    {
        cerr << "Unable to set unicast socket to reusable" << endl;
        return -1;
    }

    //begin streaming audio
    DWORD threadId;
    if ((streamThread = CreateThread(NULL, 0, playMulticastSong, NULL, 0, &threadId)) == NULL)
    {
        cerr << "Unable to create unicast thread";
        return false;
    }

    //begin reading data from the server
    receiveData();


    return 0;
}


void receiveData()
{
    while(!finished)
    {
        //receive data from the server
       int serverInfoSize = sizeof(server);
       int numReceived = 0;
       char tempBuffer[MESSAGE_SIZE];

       if ((numReceived = recvfrom(unicastSongSocket, tempBuffer, MESSAGE_SIZE,
       0, (struct sockaddr*) &server, &serverInfoSize)) < 0)
       {
           cerr << "Error reading data from unicast socket." << endl;
           continue;
       }
       //place the data into the circular buffer
       for (int i = 0; i < numReceived; i++)
       {
           uniCircBuf->buf[uniCircBuf->pos] = tempBuffer[i];
           if(uniCircBuf->pos == MUSIC_BUFFER_SIZE - 1)
           {
               uniCircBuf->pos = 0;
           }
           else
           {
               uniCircBuf->pos++;
           }
       }

    }
}

