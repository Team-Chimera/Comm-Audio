#ifndef SESSION_H_
#define SESSION_H_

#include "network.h"
#include "helper.h"
#include "../../client/controlMessage.h"

#include <string>
#include <map>
#include <sstream>
#include <fstream>
#include <vector>
#include <iostream>

#define NOMINMAX
#define FILE_PATH 64
#define SERVER_TCP_LISTEN_PORT 9874

#define SERVER_MIC_IN 9977
#define SERVER_MIC_OUT 7799

#define MAX_SESSIONS 64

typedef struct _MUSIC_SESSION {

    SOCKET_INFORMATION control;
    SOCKET_INFORMATION voice;
    SOCKET_INFORMATION send;

    char ip[IP_SIZE];

    // thread handles
    HANDLE control_thr;
    HANDLE voice_thr;
    HANDLE send_thr; // sends udp or tcp files, both won't happen at same time

    //tells send semaphore when to begin
    HANDLE sendSem;
    // tells control when send is completed so control
    // can send proper finished message.
    HANDLE sendCompleteSem;
    // Semaphore for mic sending/receiving
    HANDLE voiceSem;

    char* fileToSend;
    char filename[FILE_PATH];
    char ptp_ip[IP_SIZE];
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

extern std::string multicastSong;

/*
 *  These semaphores are for when users are added or dropped.
 * The list each client has needs to be updated, so the control
 * thread for each client needs to be updated.  userChange should
 * let all the client control threads in at once.  userAccess
 * is binary.
 *
 * */
extern HANDLE newSongSem;
extern HANDLE songAccessSem;
extern HANDLE userChangeSem;
extern HANDLE userAccessSem;
extern std::vector<std::string> userList;
extern std::vector<std::string> songList;

extern HANDLE sessionsSem;
extern std::map<SOCKET, LPMUSIC_SESSION> SESSIONS;
typedef std::pair<SOCKET, LPMUSIC_SESSION> SessionPair;

bool createSession(SOCKET c, char* a);
LPMUSIC_SESSION getSession(SOCKET s);
bool createSems(LPMUSIC_SESSION m);
bool createThreads(LPMUSIC_SESSION m);

bool initSockets(LPMUSIC_SESSION m, SOCKET control);
void sendSongList(LPMUSIC_SESSION m);

DWORD WINAPI voiceThread(LPVOID lpParameter);
DWORD WINAPI sendFileThread(LPVOID lpParameter);
DWORD WINAPI controlThread(LPVOID lpParameter);
DWORD WINAPI AcceptThread(LPVOID lpParameter);
void sendToAll(std::string message);
void sendUserList(LPMUSIC_SESSION m);
void updateNewUser(LPMUSIC_SESSION);
void sendNowPlaying(std::string, std::string, std::string, std::string);

void CALLBACK voiceRoutine(DWORD error, DWORD bytesTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags);
void CALLBACK controlRoutine(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags);
void CALLBACK sendFileRoutine(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags);

bool startSend(LPMUSIC_SESSION m, std::string filename);
void sessionCleanUp(LPMUSIC_SESSION m);



#endif
