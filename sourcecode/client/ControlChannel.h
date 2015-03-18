#ifndef CONTROLCHANNEL
#define CONTROLCHANNEL

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

#endif // CONTROLCHANNEL
