#ifndef SESSION_H_
#define SESSION_H_

#include "network.h"
#include "helper.h"

#include <string>
#include <map>
#include <sstream>
#include <fstream>
#include <vector>

#define FILE_PATH 64


typedef struct _MUSIC_SESSION {

	SOCKET_INFORMATION control;
	SOCKET_INFORMATION mic_send;
	SOCKET_INFORMATION mic_rcv;
	SOCKET_INFORMATION send;

	char ip[IP_SIZE];

	// thread handles
	HANDLE control_thr;
	HANDLE mic_send_thr;
	HANDLE mic_rcv_thr;
	HANDLE send_thr; // sends udp or tcp files, both won't happen at same time

	HANDLE sendSem;

	char* fileToSend;
	char filename[FILE_PATH];
	char mode;
	bool sending;
} MUSIC_SESSION, * LPMUSIC_SESSION;

static HANDLE sessionsSem;
extern std::map<SOCKET, LPMUSIC_SESSION> SESSIONS;
typedef std::pair<SOCKET, LPMUSIC_SESSION> SessionPair;

bool createSession(SOCKET c, char* a);
LPMUSIC_SESSION getSession(SOCKET s);
bool createSems(LPMUSIC_SESSION m);
bool createThreads(LPMUSIC_SESSION m);

bool initSockets(LPMUSIC_SESSION m, SOCKET control);

DWORD WINAPI micSendThread(LPVOID lpParameter);
DWORD WINAPI micRcvThread(LPVOID lpParameter);
DWORD WINAPI sendFileThread(LPVOID lpParameter);
DWORD WINAPI controlThread(LPVOID lpParameter);

void CALLBACK controlRoutine(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags);
// should be able to handle tcp and udp, just use sendTo and throw in the ip each time
void CALLBACK sendFileRoutine(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags);

bool startSend(LPMUSIC_SESSION m, std::string filename);

// need to clean threads and sockets


#endif