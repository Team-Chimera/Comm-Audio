#ifndef newSESSION_H_
#define newSESSION_H_

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

typedef struct _new_session
{
    SOCKET s;
    std::string ip;
    std::string song;

} new_session;

extern HANDLE userChangeSem;
extern std::vector<std::string> songList;

extern HANDLE sessionsSem;
// map has control socket and ip addr
extern std::map<SOCKET, new_session*> SESSIONS;

void AcceptThread();
void sendSongList(SOCKET c);
bool createSession(new_session* ns);
void sessionCleanUp(LPSOCKET_INFORMATION si);
DWORD WINAPI controlThread(LPVOID lpParameter);

void CALLBACK controlRoutine(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags);
void sendNowPlaying(std::string artist, std::string name, std::string album, std::string length);
void updateNewUser(SOCKET c);
void transmitSong(SOCKET s, std::string song);
DWORD WINAPI sendTCPSong(LPVOID lpParameter);



#endif