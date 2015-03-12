

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <Windows.h>

// these numbers are currently arbitrary
#define BUFFER 2048
#define DATAGRAM 256

struct SemaFunc
{
	void (*func)(char *); // pointer for the use function
	char buffer[BUFFER];  // buffer for data REMEMBER TO CHANGE THIS FROM CHAR
	int put;              // pointer to the current insertion point in the buffer
	int use;              // pointer to the current removal point in the buffer
};

void JoinMulticast();

// needs a SemaFunc struct somehow
void CALLBACK RecvMulti(DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED overlapped, DWORD flags);

void PlayMulti(SemaFunc * semaStruct);

void OutputSpeakers(char data[]); // remember to change this from char
