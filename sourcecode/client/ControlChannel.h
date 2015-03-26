#ifndef CONTROLCHANNEL
#define CONTROLCHANNEL
#define NOMINMAX

#include "controlMessage.h"

#define BUFFER_SIZE 1024
#define CONTROL_PORT 9002

#define DATA_BUFSIZE 1024
#define IP_SIZE 40

#include <WinSock2.h>
#include <windows.h>
#include <stdio.h>

#define CLIENT_TCP_PORT 6678
#define CLIENT_UDP_PORT 6698

/** Structure for Completion Routines **/
typedef struct _SOCKET_INFORMATION
{
   OVERLAPPED overlapped;
   SOCKET Socket;
   CHAR Buffer[DATA_BUFSIZE]; // used for general data, not sending whole music files.
   WSABUF DataBuf;
   DWORD BytesSEND;
   DWORD BytesRECV;
   DWORD bytesToSend;
} SOCKET_INFORMATION, * LPSOCKET_INFORMATION;

/** FUNCTIONS **/
void handleControlMessage(ctrlMessage *);
int setupControlChannel(hostent *);
void parseControlString(std::string, ctrlMessage *);
DWORD WINAPI read(LPVOID);
void CALLBACK ReadRoutine(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);

/** GUI editors **/
void updateListeners(std::vector<std::string>);
void updateNowPlaying(std::vector<std::string> msgData);
void establishGUIConnector(void *);

#endif // CONTROLCHANNEL
