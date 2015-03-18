/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: voiceChat.h
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

#ifndef VOICECHAT_H
#define VOICECHAT_H

#include "client.h"
#include "circularbuffer.h"

struct SOCKET_INFORMATION
{
   OVERLAPPED Overlapped;
   SOCKET Socket;
   CircularBuffer * cBuffer;
   WSABUF DataBuf;
   DWORD BytesSEND;
   DWORD BytesRECV;
};

struct BufControl
{
	char buffer[BUFFER];  // buffer for data REMEMBER TO CHANGE THIS FROM CHAR
	int put;              // pointer to the current insertion point in the buffer
	int use;              // pointer to the current removal point in the buffer
};

#endif
