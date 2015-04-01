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

typedef struct _MULTICAST_INFORMATION {
   OVERLAPPED Overlapped;
   SOCKET Socket;
   SOCKADDR_IN Client;
   struct ip_mreq StMreq;
   CHAR Buffer[DATA_BUFSIZE];
   WSABUF DataBuf;
   DWORD BytesSent;
} MULTICAST_INFORMATION, *LPMULTICAST_INFORMATION;

DWORD WINAPI startMulticastThread(LPVOID lpParameter);
LPMULTICAST_INFORMATION initMulticastSocket();
DWORD WINAPI multicastSendLoop(LPVOID lpParameter);
void CALLBACK multicastRoutine(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags);

void displayError(char *errStr, int errCode);

#endif
