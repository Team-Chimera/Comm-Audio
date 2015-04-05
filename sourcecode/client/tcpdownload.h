#ifndef TCPDOWNLOADSS
#define TCPDOWNLOADSS

#define CLIENT_DOWNLOAD_PORT 6678

#include <vector>
#include <deque>
#include <string>
#include <sstream>
#include <fstream>
#include <ctime>

#include <WinSock2.h>
#include <Windows.h>
#include "socketinfo.h"

bool openListenSocket(SOCKET* s, int port);
DWORD WINAPI doTCPDownload(LPVOID lpParameter);
void CALLBACK tcpDownloadRoutine(DWORD Error, DWORD BytesTransferred,
LPWSAOVERLAPPED Overlapped, DWORD InFlags);
void deleteSocketInfo(LPSOCKET_INFORMATION si);

bool createWorkerThread(LPTHREAD_START_ROUTINE routine, HANDLE* hThread, LPVOID param, DWORD flags);
char *readTCP(SOCKET* s, int size);

#endif // TCPDOWNLOAD

