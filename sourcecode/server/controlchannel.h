#ifndef CONTROLCHANNEL
#define CONTROLCHANNEL

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <list>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <winsock2.h>
#include <winnetwk.h>
#include <ws2spi.h>
#include <wtsapi32.h>

#define CONTROL_LISTEN_PORT 5000
#define CONTROL_DATA_PORT 5002

#endif // CONTROLCHANNEL

void openControlListener();
DWORD WINAPI acceptClients(LPVOID arg);
int createNewClient();
