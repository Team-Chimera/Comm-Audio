/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: multicast.h
--
-- PROGRAM: CommAudio_Client
--
-- DATE: March 12, 2015
--
-- REVISIONS: Created March 10, 2015
--
-- DESIGNER: Michael Chimick
--
-- PROGRAMMER: Michael Chimick
--
-- NOTES:
--
--
----------------------------------------------------------------------------------------------------------------------*/

#ifndef MULTICAST_H
#define MULTICAST_H

#include "client.h"
#include "circularbuffer.h"

struct SOCKET_INFORMATION
{
   OVERLAPPED overlapped;
   SOCKET socket;
   CircularBuffer * cBuffer;
   WSABUF datagram;
   DWORD bytesRECV;
   struct ip_mreq addr;
};

void InitializeMulticastData();

void JoinMulticast(SOCKET s, OVERLAPPED o, in_addr group, in_addr local);
void DropMulticast();

DWORD WINAPI RecvMultiThread(LPVOID parameter);
void CALLBACK RecvMulti(DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags);

DWORD WINAPI PlayMultiThread(LPVOID parameter);
void PlayMulti(CircularBuffer * cBuffer);

void OutputSpeakers(byte * data, int size);

#endif
