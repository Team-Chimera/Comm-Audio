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

void JoinMulticast();

DWORD WINAPI RecvMultiThread(LPVOID parameter);
void CALLBACK RecvMulti(DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags);

DWORD WINAPI PlayMultiThread(LPVOID parameter);
void PlayMulti(CircularBuffer * cBuffer);

void OutputSpeakers(byte * data, int size); // remember to change this from char

#endif
