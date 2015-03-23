/*---------------------------------------------------------------------------------------------
-- SOURCE FILE: network.cpp -
--
-- PROGRAM:
--
-- DATE: March 22, 2015
--
-- Interface:
        SOCKET createTCPSOCKET();
        SOCKET createUDPSOCKET();
        bool connectTCPSOCKET(SOCKET s, sockaddr_in* addr);
        bool createSocketInfo(LPSOCKET_INFORMATION si, SOCKET s);
        void deleteSocketInfo(LPSOCKET_INFORMATION si);

        bool initWSA(WSADATA* wsaData);
        bool openListenSocket(SOCKET* s, int port);
        bool setAcceptEvent(WSAEVENT* a);

        bool getIP_Addr(sockaddr_in* addr, char* host, int port);
        void printIP(sockaddr_in& addr);
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Jeff Bayntun
--
-- PROGRAMMER: Jeff Bayntun
--
--------------------------------------------------------------------------------------------*/

#include "network.h"

const char* DENY = "NO!";
/*******************************************************************
** Function: createTCPSOCKET()
**
** Date: March 22th, 2015
**
** Revisions:
**
**
** Designer: Jeff Bayntun
**
** Programmer: Jeff Bayntun
**
** Interface:
**			SOCKET createTCPSOCKET()
**
**
** Returns:
**			socket value, -1 on failure
**
** Notes:
*******************************************************************/
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
/*******************************************************************
** Function: createUDPSOCKET()
**
** Date: March 22th, 2015
**
** Revisions:
**
**
** Designer: Jeff Bayntun
**
** Programmer: Jeff Bayntun
**
** Interface:
**			SOCKET createUDPSOCKET()
**
**
** Returns:
**			socket value, -1 on failure
**
** Notes:
*******************************************************************/
SOCKET createUDPSOCKET()
{
    SOCKET s;
    if ((s = WSASocket(AF_INET, SOCK_DGRAM, 0, NULL, 0,
      WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
    {
        printf("Failed to get a udp socket %d\n", WSAGetLastError());
        return -1;
    }
    return s;
}
/*******************************************************************
** Function: connectTCPSOCKET()
**
** Date: March 22th, 2015
**
** Revisions:
**
**
** Designer: Jeff Bayntun
**
** Programmer: Jeff Bayntun
**
** Interface:
**			bool connectTCPSOCKET(SOCKET s, sockaddr_in* addr)
**          s:  socket to connect
**          addr: structure with ip and port to connect to
**
**
** Returns:
**			false on failure
**
** Notes:
*******************************************************************/
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
/*******************************************************************
** Function: printIP()
**
** Date: March 22th, 2015
**
** Revisions:
**
**
** Designer: Jeff Bayntun
**
** Programmer: Jeff Bayntun
**
** Interface:
**			void printIP(sockaddr_in& peer)
**
**
** Returns:
**			void
**
** Notes: prints the ip address from a sockaddr_in structure
*******************************************************************/
void printIP(sockaddr_in& peer)
{
    char* temp_addr = inet_ntoa( peer.sin_addr );
    printf("IP address is %s\n", temp_addr);

}
/*******************************************************************
** Function: createSocketInfo()
**
** Date: March 22th, 2015
**
** Revisions:
**
**
** Designer: Jeff Bayntun based on code from Aman Abdulla
**
** Programmer: Jeff Bayntun
**
** Interface:
**			bool createSocketInfo(LPSOCKET_INFORMATION si, SOCKET s)
**          si:     pointer to socket information to populate
**          s:      socket to use for this socket info
**
**
** Returns:
**			true on success
**
** Notes: sets default values for a SOCKET_INFORMATION
*******************************************************************/
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
/*******************************************************************
** Function: deleteSocketInfo()
**
** Date: March 22th, 2015
**
** Revisions:
**
**
** Designer: Jeff Bayntun
**
** Programmer: Jeff Bayntun
**
** Interface:
**			void deleteSocketInfo(LPSOCKET_INFORMATION si)
**          si:     pointer to socket information to populate
**
**
** Returns:
**			void
**
** Notes:
*******************************************************************/
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
/*******************************************************************
** Function: initWSA()
**
** Date: March 22th, 2015
**
** Revisions:
**
**
** Designer: Jeff Bayntun
**
** Programmer: Jeff Bayntun
**
** Interface:
**			bool initWSA(WSADATA* wsaData)
**          wsaData: pointer to WSADATA to initialize
**
**
** Returns:
**			false on failure
**
** Notes:
*******************************************************************/
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
/*******************************************************************
** Function: getIP_Addr()
**
** Date: March 22th, 2015
**
** Revisions:
**
**
** Designer: Jeff Bayntun
**
** Programmer: Jeff Bayntun
**
** Interface:
**			bool getIP_Addr(sockaddr_in* addr, char* host, int port)
**          addr:   out param, will be filled with host and port
**          host:   pointer to char for host name
**          port:   port to use
**
**
** Returns:
**			false on failure
**
** Notes:
*******************************************************************/
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
