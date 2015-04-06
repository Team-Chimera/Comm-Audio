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

struct MetaData;

typedef struct _new_session
{
    SOCKET s;
    std::string ip;
    std::string song;
    HANDLE transferCompleteSem;

} new_session;

extern std::vector<std::string> songList;

extern HANDLE sessionsSem;
// map has control socket and ip addr
extern std::map<SOCKET, new_session*> SESSIONS;

void AcceptThread();
void sendSongList(SOCKET c);
bool createSession(new_session* ns);
void sessionCleanUp(SOCKET s);
DWORD WINAPI controlThread(LPVOID lpParameter);

void CALLBACK controlRoutine(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags);
void sendNowPlaying(MetaData *);
void updateNewUser(SOCKET c);
void transmitSong(SOCKET s, std::string song);
DWORD WINAPI sendTCPSong(LPVOID lpParameter);
void sendToAll(std::string);
void sendUserList(SOCKET c);
void sendSongDone(SOCKET s);


#endif