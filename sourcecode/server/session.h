#ifndef SESSION_H_
#define SESSION_H_

#include "network.h"
#include "helper.h"
#include "../client/controlMessage.h"

#include <string>
#include <map>
#include <sstream>
#include <fstream>
#include <vector>

#define FILE_PATH 64
#define SERVER_TCP_LISTEN_PORT 9874

#define SERVER_MIC_IN 9977
#define SERVER_MIC_OUT 7799

#define MAX_SESSIONS 2000

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

    //tells send semaphore when to begin
    HANDLE sendSem;
    // tells control when send is completed so control
    // can send proper finished message.
    HANDLE sendCompleteSem;

    char* fileToSend;
    char filename[FILE_PATH];
    char mode;
    bool sending;
} MUSIC_SESSION, * LPMUSIC_SESSION;

/*
 *  new song sem tells control sockets to send the name of
 * the new song to the clients.  It should have a high max,
 * and when there is a new song, it can let all the songs in
 * simultaneously.
 *
 * Song access is a binary semaphore so that song name isn't
 * changed while other threads are reading it
 * */
static HANDLE newSongSem;
static HANDLE songAccessSem;
static std::string multicastSong;

/*
 *  These semaphores are for when users are added or dropped.
 * The list each client has needs to be updated, so the control
 * thread for each client needs to be updated.  userChange should
 * let all the client control threads in at once.  userAccess
 * is binary.
 *
 * */
static HANDLE userChangeSem;
static HANDLE userAccessSem;
static std::vector<std::string> userList;

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
DWORD WINAPI AcceptThread(LPVOID lpParameter);

void CALLBACK controlRoutine(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags);
void CALLBACK sendFileRoutine(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags);

bool startSend(LPMUSIC_SESSION m, std::string filename);
void sessionCleanUp(LPMUSIC_SESSION m);


#endif
