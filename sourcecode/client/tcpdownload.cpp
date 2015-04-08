#include <iostream>
#include "tcpdownload.h"
#include "controlChannel.h"

using namespace std;
bool TCP_rcv;


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: TestThread
--
-- DATE: January 30, 2015
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Jeff Bayntun
--
-- PROGRAMMER: Jeff Bayntun
--
-- INTERFACE: DWORD WINAPI TestThread(LPVOID lpParameter)
-- lpParameter: points to a WSA event to wait for
--
-- RETURNS: false on failure
--
-- NOTES:
    listens for a test connection and sets up the correct WSA receive event
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
-- FUNCTION: tcpDownloadRotuine
--
-- DATE: January 30, 2015
--
-- REVISIONS
--
-- DESIGNER: Jeff Bayntun
--
-- PROGRAMMER: Jeff Bayntun
--
-- INTERFACE: void CALLBACK tcpDownloadRoutine(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags)
--
-- RETURNS: void
--
-- NOTES:
    callback function for test WSA receive events
----------------------------------------------------------------------------------------------------------------------*/
void CALLBACK tcpDownloadRoutine(DWORD Error, DWORD BytesTransferred,
   LPWSAOVERLAPPED Overlapped, DWORD InFlags)
{
   DWORD RecvBytes;
   DWORD Flags;

   string message_rcv;
   // Reference the WSAOVERLAPPED structure as a SOCKET_INFORMATION structure
   LPSOCKET_INFORMATION SI = (LPSOCKET_INFORMATION) Overlapped;

   if (Error != 0)
   {
     printf("I/O operation failed with error %d\n", Error);
   }

   if (BytesTransferred == 0) // nothing sent or rcvd...
   {
      printf("Download completed %d\n", SI->socket);
   }

   if (Error != 0 || BytesTransferred == 0)
   {
      TCP_rcv = false;
      return;
   }

   // Check to see if the BytesRECV field equals zero. If this is so, then
   // this means a WSARecv call just completed so update the BytesRECV field
   // with the BytesTransferred value from the completed WSARecv() call.

   if (SI->bytesRECV == 0) // after a rcv
   {
      SI->bytesRECV = BytesTransferred;
      SI->bytesSEND = 0;
      //rcv = true;
      message_rcv.assign(SI->Buffer);

    //  file << message_rcv;

   }

      SI->bytesRECV = 0;

      // Now that there are no more bytes to send post another WSARecv() request.

      Flags = 0;
      ZeroMemory(&(SI->overlapped), sizeof(WSAOVERLAPPED));

      SI->DataBuf.len = DATA_BUFSIZE;
      SI->DataBuf.buf = SI->Buffer; // should this be zeroed??


      if (WSARecv(SI->socket, &(SI->DataBuf), 1, &RecvBytes, &Flags,
         &(SI->overlapped), tcpDownloadRoutine) == SOCKET_ERROR)
      {
         if (WSAGetLastError() != WSA_IO_PENDING )
         {
            printf("TestWSARecv() failed with error %d\n", WSAGetLastError());
            return;
         }
      }
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
