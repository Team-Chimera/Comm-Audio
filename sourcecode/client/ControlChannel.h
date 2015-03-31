#ifndef CONTROLCHANNEL
#define CONTROLCHANNEL
#define NOMINMAX

#include "controlMessage.h"

#define BUFFER_SIZE 1024
#define CONTROL_PORT 9874

#define IP_SIZE 40

#include <WinSock2.h>
#include <stdio.h>

#define CLIENT_TCP_PORT 6678
#define CLIENT_UDP_PORT 6698


/** FUNCTIONS **/
int setupControlChannel(hostent *);
DWORD WINAPI read(LPVOID);
void CALLBACK ReadRoutine(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
bool downloadSong(std::string);
bool requestSong(std::string);

void handleControlMessage(ctrlMessage *);
void createControlString(ctrlMessage, std::string &);
void parseControlString(std::string, ctrlMessage *);

/** GUI editors **/
void updateListeners(std::vector<std::string>);
void updateNowPlaying(std::vector<std::string> msgData);
void establishGUIConnector(void *);



#endif // CONTROLCHANNEL
