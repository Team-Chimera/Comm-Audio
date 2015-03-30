
#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <string>
#include "controlMessage.h"
#include "controlChannel.h"
#include "unicastSong.h"

using namespace std;

//UDP data socket
SOCKET unicastSongSocket;


/*******************************************************************
** Function: uncastSong
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
    //conver tthe parameter
    hostent *hp = (hostent *) host;

    //set up the structure
    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(CLIENT_UNICAST_PORT);


    // Copy the server address from the resolved host
    memcpy((char *)&server.sin_addr, hp->h_addr, hp->h_length);

    //create the UDP socket
    if ((unicastSongSocket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
    {
        cerr << "Unable to creat the Unicast UDP socket." << endl;
        return -1;
    }

    return 0;
}


