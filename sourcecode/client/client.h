/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: multicast.h
--
-- PROGRAM: CommAudio_Client
--
-- DATE: March 12, 2015
--
-- REVISIONS: Created March 10, 2015
--
-- DESIGNER: Michael Chimick
--
-- PROGRAMMER: Michael Chimick
--
-- NOTES:
--
--
----------------------------------------------------------------------------------------------------------------------*/
#ifndef CLIENT_H
#define CLIENT_H

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#pragma comment (lib, "Ws2_32.lib")

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <winsock2.h>
#include <winnetwk.h>
#include <ws2spi.h>
#include <wtsapi32.h>

// these numbers are currently arbitrary
#define BUFFER 2048
#define DATAGRAM 256

struct Semaphores
{
    HANDLE semaBuf;
    HANDLE semaIn;
    HANDLE semaOut;
};

#endif
