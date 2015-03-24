#ifndef CONTROLCHANNEL
#define CONTROLCHANNEL
#define NOMINMAX

#include "controlMessage.h"

#define BUFFER_SIZE 1024
#define CONTROL_PORT 9002

/** FUNCTIONS **/
void handleControlMessage(ctrlMessage *);
int setupControlChannel(hostent *);
void parseControlString(std::string, ctrlMessage *);
DWORD WINAPI read(LPVOID);
void CALLBACK ReadRoutine(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);

/** GUI editors **/
void updateListeners(std::vector<std::string>);
void updateNowPlaying(std::vector<std::string> msgData);

#endif // CONTROLCHANNEL
