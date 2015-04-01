#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <fstream>
#include <string>
#include "socketinfo.h"
#include "downloadSong.h"

using namespace std;

//socket for receiving data on
SOCKET downloadSocket;
SOCKET listenSocket;
SOCKET_INFORMATION downloadSocketInfo;

bool downloading = false;
//file to save to
ofstream songFile;

/*******************************************************************
** Function: beginDownload
**
** Date: March 31st, 2015
**
** Revisions:
**
**
** Designer: Rhea Lauzon
**
** Programmer: Rhea Lauzon
**
** Interface:
**			DWORD WINAPI beginDownload(LPVOID song)
**              LPVOID song -- Song name to receive
**
** Returns:
**			DWORD -- -1 on failure of creating UDP socket,
**                      0 on success
**
** Notes:
** Creates the Unicast UDP socket and begins reading song data.
**
*******************************************************************/
DWORD WINAPI beginDownload(LPVOID songName)
{
    SOCKADDR_IN InternetAddr;

    string song = (char *) songName;

    //create the song
    if ((listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
    {
         printf("Failed to get a socket %d\n", WSAGetLastError());
         return -1;
    }

    //assign the location to it
    InternetAddr.sin_family = AF_INET;
    InternetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    InternetAddr.sin_port = htons(CLIENT_DOWNLOAD_PORT);

    //bind the listening socket
    if (bind(listenSocket, (PSOCKADDR) &InternetAddr, sizeof(InternetAddr)) == SOCKET_ERROR)
    {
        printf("bind() failed with error %d\n", WSAGetLastError());
        return -1;
    }

    //set the socket into listening
    if (listen(listenSocket, 5))
    {
      printf("listen() failed with error %d\n", WSAGetLastError());
      return -1;
    }

    //create a file
    songFile.open(song.c_str(), ios::binary);

    if (!songFile.is_open())
    {
        cerr << "Unable to create that file" << endl;
        return -1;
    }

    //wait for the server to connect
    downloadSocket = accept(listenSocket, NULL, NULL);

    //close the listener now that the server is connected
    closesocket(listenSocket);

    //begin reading data
    download();

    return 0;
}

int download()
{
    downloading = true;
    DWORD bytesReceived = 0;
    int index;
    DWORD flags = 0;
    WSAEVENT eventArray[1];

    //set the socket structure
    ZeroMemory(&(downloadSocketInfo.overlapped), sizeof(WSAOVERLAPPED));
    downloadSocketInfo.DataBuf.len = DATA_BUFSIZE;
    downloadSocketInfo.DataBuf.buf = downloadSocketInfo.Buffer;

    while(downloading)
    {
        if (WSARecv(downloadSocket, &(downloadSocketInfo.DataBuf), 1, &bytesReceived, &flags, &(downloadSocketInfo.overlapped), downloadReadRoutine) == SOCKET_ERROR)
        {
            if (WSAGetLastError() != WSA_IO_PENDING)
            {
                songFile.close();
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
** Function: downloadReadRoutine
**
** Date: March 31st, 2015
**
** Revisions:
**
**
** Designer: Rhea Lauzon
**
** Programmer: Rhea Lauzon
**
** Interface:
**			void CALLBACK downloadReadRoutine(DWORD Error,
**                  DWORD bytesReceived, LPWSAOVERLAPPED Overlapped, DWORD inFlags)
**
** Returns:
**			void
**
** Notes:
** Completion routine of the downloading of a song.
**
*******************************************************************/
void CALLBACK downloadReadRoutine(DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD inFlags)
{
    DWORD recvBytes;
    DWORD flags;

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
        closesocket(downloadSocket);
        songFile.close();
        return;
    }

    cout << "Received: " << sockInfo->Buffer << endl;

    songFile << sockInfo->Buffer;
    cout.flush();
    Sleep(10);

}
