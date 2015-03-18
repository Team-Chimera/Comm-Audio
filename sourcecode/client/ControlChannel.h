#ifndef CONTROLCHANNEL
#define CONTROLCHANNEL


#define BUFFER_SIZE 1024
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>
#include <vector>
#include <list>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <winsock2.h>
#include <winnetwk.h>
#include <ws2spi.h>
#include <wtsapi32.h>
#include "controlMessage.h"

/** FUNCTIONS **/
void handleControlMessage(ctrlMessage *);
int setupControlChannel(int , hostent *);
void parseControlString(std::string, ctrlMessage *);
DWORD WINAPI read(LPVOID);
void CALLBACK ReadRoutine(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);

/** GUI editors **/
void updateListeners(std::vector<std::string>);

#endif // CONTROLCHANNEL
