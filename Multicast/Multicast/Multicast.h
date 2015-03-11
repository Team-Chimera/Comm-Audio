#ifndef MULTICAST_H
#define MULTICAST_H

#include <WinSock2.h>
#include <ws2tcpip.h>
#include <cstdio>

#define DATA_BUFSIZE 1024
#define TIMECAST_ADDR   "234.5.6.7"
#define TIMECAST_PORT   8910
#define TIMECAST_TTL    0
#define TIMECAST_INTRVL 1

typedef struct _SOCKET_INFORMATION {
   OVERLAPPED Overlapped;
   SOCKET Socket;
   SOCKADDR_IN Server;
   SOCKADDR_IN Client;
   CHAR Buffer[DATA_BUFSIZE];
   WSABUF DataBuf;
   DWORD BytesSEND;
   DWORD BytesRECV;
} SOCKET_INFORMATION, *LPSOCKET_INFORMATION;

void CALLBACK WorkerRoutine(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags);

#endif