#ifndef comm_audio_server_h
#define comm_audio_server_h

#include <Windows.h>
#include <string>

#define SERVER_TCP_LISTEN_PORT 9874

HANDLE acceptThread;

DWORD WINAPI AcceptThread(LPVOID lpParameter);
void cleanUp();

#endif