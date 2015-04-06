#ifndef CONTROLCHANNEL
#define CONTROLCHANNEL
#define NOMINMAX

#include "controlMessage.h"

#define BUFFER_SIZE 1024
#define CONTROL_PORT 9874

#include <WinSock2.h>
#include <stdio.h>

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
void updateNowPlaying(std::vector<std::string>);
void establishGUIConnector(void *);
void updateLibrary(std::vector<std::string>);



#endif // CONTROLCHANNEL
