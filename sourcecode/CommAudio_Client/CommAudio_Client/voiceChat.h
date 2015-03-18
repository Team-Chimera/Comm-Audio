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
	DWORD bytesSEND;
	struct ip_mreq addr;
};

void InitVoiceData();

void StartVoiceChat(SOCKET s, OVERLAPPED o, in_addr group, in_addr local);
void DropMulticast();

DWORD WINAPI RecvVoiceThread(LPVOID parameter);
void CALLBACK RecvVoice(DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags);

DWORD WINAPI PlayVoiceThread(LPVOID parameter);
void PlayVoice();

void VoiceOutputSpeakers(byte * data, int size);

DWORD WINAPI RecordVoiceThread(LPVOID parameter);
void RecordVoice();

DWORD WINAPI SendVoiceThread(LPVOID parameter);
void SendVoice();

#endif
