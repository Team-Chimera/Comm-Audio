#include "stdafx.h"

#include "network.h"

const char* DENY = "NO!";

SOCKET createTCPSOCKET()
{
	SOCKET s;
	if ((s = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, 
      WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET) 
	{
		printf("Failed to get a tcp socket %d\n", WSAGetLastError());
		return -1;
	}
	return s;
}
SOCKET createUDPSOCKET()
{
	SOCKET s;
	if ((s = WSASocket(AF_INET, SOCK_DGRAM, 0, NULL, 0, 
      WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET) 
	{
		printf("Failed to get a tcp socket %d\n", WSAGetLastError());
		return -1;
	}
	return s;
}

bool connectTCPSOCKET(SOCKET s, sockaddr_in* addr)
{

	if (connect(s, (SOCKADDR*)addr, sizeof(sockaddr_in)) == SOCKET_ERROR)
    {
        perror("failed to connect socket.\n");
		printf("wsa get error  %d\n", WSAGetLastError());
        return false;
    }
	return true;
}

void printIP(sockaddr_in& peer)
{
	char* temp_addr = inet_ntoa( peer.sin_addr );
	printf("IP address is %s\n", temp_addr);

}

bool createSocketInfo(LPSOCKET_INFORMATION si, SOCKET s)
{
	ZeroMemory(si, sizeof(SOCKET_INFORMATION));
	ZeroMemory(&(si->Overlapped), sizeof(WSAOVERLAPPED));

	  si->Socket = s; 
      si->BytesSEND = 0;
      si->BytesRECV = 0;
	  si->bytesToSend = 0;
      si->DataBuf.len = DATA_BUFSIZE;
      si->DataBuf.buf = si->Buffer;

	  return true;
}

void deleteSocketInfo(LPSOCKET_INFORMATION si)
{
	if (closesocket(si->Socket) == SOCKET_ERROR)
         {
            printf("closesocket() failed with error %d\n", WSAGetLastError());
         }

         GlobalFree(si);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: openListenSocket(SOCKET* s, int port)
--
-- DATE: January 30, 2015
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Jeff Bayntun
--
-- PROGRAMMER: Jeff Bayntun
--
-- INTERFACE: bool openListenSocket(SOCKET* s, int port)
-- s: pointer to a socket 
-- port: port to bind socket with
--			
-- RETURNS: false if fails
--
-- NOTES:
----------------------------------------------------------------------------------------------------------------------*/
bool openListenSocket(SOCKET* s, int port)
{
	SOCKADDR_IN InternetAddr;

	if ((*s = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, 
      WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET) 
	{
		printf("Failed to get a socket %d\n", WSAGetLastError());
		return false;
	}

   InternetAddr.sin_family = AF_INET;
   InternetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
   InternetAddr.sin_port = htons(port);

   if (bind(*s, (PSOCKADDR) &InternetAddr,
      sizeof(InternetAddr)) == SOCKET_ERROR)
   {
      printf("bind() failed with error %d\n", WSAGetLastError());
      return false;
   }

   if (listen(*s, 5))
   {
      printf("listen() failed with error %d\n", WSAGetLastError());
      return false;
   }

   return true;
}

bool initWSA(WSADATA* wsaData)
{
	int Ret;

	if ((Ret = WSAStartup(0x0202, wsaData)) != 0)
	{
		printf("WSAStartup failed with error %d\n", Ret);
		WSACleanup();
		return false;
	}

	return true;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: setAcceptEvent
--
-- DATE: January 30, 2015
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Jeff Bayntun
--
-- PROGRAMMER: Jeff Bayntun
--
-- INTERFACE: bool setAcceptEvent(WSAEVENT* a)
-- a: pointer to the event to create
--			
-- RETURNS: false if fails
--
-- NOTES:
----------------------------------------------------------------------------------------------------------------------*/
bool setAcceptEvent(WSAEVENT* a)
{
	if ((*a = WSACreateEvent()) == WSA_INVALID_EVENT)
	{
		printf("WSACreateEvent() failed with error %d\n", WSAGetLastError());
		return false;
	}
	return true;
}

bool getIP_Addr(sockaddr_in* addr, char* host, int port)
{
	struct hostent	*hp;
	
   if ((hp = gethostbyname(host)) == NULL)
	{
		printf("Unknown server address\n");
		return false;
	}

   ZeroMemory(addr, sizeof(sockaddr_in));
   addr->sin_family = AF_INET;
   addr->sin_port = htons(port);

   memcpy((char *)&addr->sin_addr, hp->h_addr, hp->h_length);

   return true;
}