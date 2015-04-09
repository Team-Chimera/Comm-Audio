/*******************************************************************
** File: tcpdownload.cpp
**
** Program: client.exe
**
** Date: April 1, 2015
**
** Revisions:
**
**
** Designer: Jeff Bayntun
**
** Programmer: Jeff Bayntun
**
** Interface:
**        bool openListenSocket(SOCKET* s, int port);
**        DWORD WINAPI doTCPDownload(LPVOID lpParameter);
**        void CALLBACK tcpDownloadRoutine(DWORD Error, DWORD BytesTransferred,
**        LPWSAOVERLAPPED Overlapped, DWORD InFlags);
**        void deleteSocketInfo(LPSOCKET_INFORMATION si);
**        bool createWorkerThread(LPTHREAD_START_ROUTINE routine, HANDLE* hThread, LPVOID param, DWORD flags);
**        void readTCP(SOCKET* s, int size, char** out , int *size_out);
**
**
** Notes:
** Handles the TCP download of a song from the server
**
*******************************************************************/

#include <iostream>
#include "tcpdownload.h"
#include "controlChannel.h"

using namespace std;
bool TCP_rcv;


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: doTCPDownload
--
-- DATE: Date: April 1, 2015
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Jeff Bayntun
--
-- PROGRAMMER: Jeff Bayntun
--
-- INTERFACE: DWORD WINAPI doTCPDownload(LPVOID lpParameter)
--          lpParameter: string* to name of file
--
-- RETURNS: FALSE on failure
--
-- NOTES:
    opens a file to store data in, opens a listen socket and accepts a connection,
    then stores the data in file as it is received.  Closes file and socket.
----------------------------------------------------------------------------------------------------------------------*/
DWORD WINAPI doTCPDownload(LPVOID lpParameter)
{
   string* file_name = (std::string*) lpParameter;

   ofstream file;
   SOCKET acceptSocket;

   file.open(*file_name, ios::binary);

   if(!file.is_open())
   {
       std::cout << "error opening file, exiting file download" << std::endl;
       fflush(stdout);
       delete file_name;
       return FALSE;
   }

   //create a tcp socket to wait for a connection on
   openListenSocket(&acceptSocket, CLIENT_DOWNLOAD_PORT);
   SOCKET downloadSocket = accept(acceptSocket, NULL, NULL);

   char* incoming;
   int size_out;

      // start making rcv calls
   while(1)
   {
       readTCP(&downloadSocket,BUFFER_SIZE, &incoming, &size_out);
       if(!size_out)
       {
           break;
       }
       file.write(incoming, size_out);
       delete [] incoming;
   }

    delete file_name;
      closesocket(downloadSocket);
      closesocket(acceptSocket);
      file.close();


   return TRUE;
}


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: openListenSocket(SOCKET* s, int port)
--
-- DATE: January 30, 2015
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Jeff Bayntun
--
-- PROGRAMMER: Jeff Bayntun
--
-- INTERFACE: bool openListenSocket(SOCKET* s, int port)
-- s: pointer to a socket
-- port: port to bind socket with
--
-- RETURNS: false if fails
--
-- NOTES:
----------------------------------------------------------------------------------------------------------------------*/
bool openListenSocket(SOCKET* s, int port)
{
    SOCKADDR_IN InternetAddr;

    if ((*s = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0,
      WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
    {
        printf("Failed to get a socket %d\n", WSAGetLastError());
        return false;
    }

   InternetAddr.sin_family = AF_INET;
   InternetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
   InternetAddr.sin_port = htons(port);

   if (bind(*s, (PSOCKADDR) &InternetAddr,
      sizeof(InternetAddr)) == SOCKET_ERROR)
   {
      printf("bind() failed with error %d\n", WSAGetLastError());
      return false;
   }

   if (listen(*s, 5))
   {
      printf("listen() failed with error %d\n", WSAGetLastError());
      return false;
   }

   return true;
}


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: deleteSocketInfo
--
-- DATE: January 30, 2015
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Jeff Bayntun
--
-- PROGRAMMER: Jeff Bayntun
--
-- INTERFACE: void deleteSocketInfo(LPSOCKET_INFORMATION si)
-- si: information about the socket
--
-- RETURNS: void
--
-- NOTES: Deletes the socket information structure
----------------------------------------------------------------------------------------------------------------------*/
void deleteSocketInfo(LPSOCKET_INFORMATION si)
{
    if (closesocket(si->socket) == SOCKET_ERROR)
         {
            printf("closesocket() failed with error %d\n", WSAGetLastError());
         }

         GlobalFree(si);
}



/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: createWorkerThread
--
-- DATE: January 18, 2015
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Jeff Bayntun
--
-- PROGRAMMER: Jeff Bayntun
--
-- INTERFACE: bool createWorkerThread(LPTHREAD_START_ROUTINE routine, HANDLE* hThread, LPVOID param, DWORD flags)
--  routine: function this thread will start in
    hTHread: out param, handle value for this thread
    param: void* to pass to thread
    flats: any flags for thread creation
--
-- RETURNS: false on failure to create thread, else true
--
-- NOTES:
----------------------------------------------------------------------------------------------------------------------*/

bool createWorkerThread(LPTHREAD_START_ROUTINE routine, HANDLE* hThread, LPVOID param, DWORD flags)
{
    if ((*hThread = CreateThread(NULL, 0, routine, param, flags, 0)) == NULL)
    {
        printf("CreateThread failed with error %d\n", GetLastError());
        return false;
    }
    return true;
}


/*-------------------------------------------------------------------------------------------------
-- FUNCTION: readTCP
--
-- DATE: January 30, 2015
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Jeff Bayntun
--
-- PROGRAMMER: Jeff Bayntun
--
-- INTERFACE: string readTCP(SOCKET* s, int size)
-- s: pointer to SOCKET to read from
-- size: size of message to expect, defaults to BUFSIZE
--
-- RETURNS: string containing the message read
--
-- NOTES:
-------------------------------------------------------------------------------------------------*/
void readTCP(SOCKET* s, int size, char** out, int* size_out )
{
    char* buf = new char[size];
    *size_out = recv(*s, buf, size, 0);

   // std::cout << buf << endl;
    *out = buf;
}
