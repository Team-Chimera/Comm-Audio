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

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#pragma comment (lib, "Ws2_32.lib")

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <Windows.h>

// these numbers are currently arbitrary
#define BUFFER 2048
#define DATAGRAM 256