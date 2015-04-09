#ifndef NETWORK_H_
#define NETWORK_H_

#define NOMINMAX

#define DATA_BUFSIZE 1024
#define IP_SIZE 40

#include <WinSock2.h>
#include <windows.h>
#include <stdio.h>
#include <string>
#include <iostream>

#define CLIENT_TCP_PORT 6678
#define CLIENT_UDP_PORT 6698

typedef struct _SOCKET_INFORMATION {
   OVERLAPPED Overlapped;
   SOCKET Socket;
   CHAR Buffer[DATA_BUFSIZE]; // used for general data, not sending whole music files.
   WSABUF DataBuf;
   DWORD BytesSEND;
   DWORD BytesRECV;
   DWORD bytesToSend;
} SOCKET_INFORMATION, * LPSOCKET_INFORMATION;

extern const char* DENY;


SOCKET createTCPSOCKET();
SOCKET createUDPSOCKET();
bool connectTCPSOCKET(SOCKET s, sockaddr_in* addr);
LPSOCKET_INFORMATION createSocketInfo(SOCKET s);
void deleteSocketInfo(LPSOCKET_INFORMATION si);

bool initWSA(WSADATA* wsaData);
bool openListenSocket(SOCKET* s, int port);
bool setAcceptEvent(WSAEVENT* a);
int sendTCPMessage( SOCKET* s, std::string message, int size);
int sendTCPMessage( SOCKET* s, char* message, long file_size, int packet_size);

bool getIP_Addr(sockaddr_in* addr, char* host, int port);
void printIP(sockaddr_in& addr);
bool openTCPSend( SOCKET* s, int port, std::string ip);





#endif
