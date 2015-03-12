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

#include "client.h"

struct BufControl
{
	char buffer[BUFFER];  // buffer for data REMEMBER TO CHANGE THIS FROM CHAR
	int put;              // pointer to the current insertion point in the buffer
	int use;              // pointer to the current removal point in the buffer
};

struct 
{

};

void JoinMulticast();

DWORD WINAPI RecvMultiThread(LPVOID parameter);
void CALLBACK RecvMulti(DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags);

DWORD WINAPI PlayMultiThread(LPVOID parameter);
void PlayMulti(BufControl * bCont);

void OutputSpeakers(char data[]); // remember to change this from char
