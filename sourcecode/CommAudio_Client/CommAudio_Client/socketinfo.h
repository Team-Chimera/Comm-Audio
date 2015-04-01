#ifndef SOCKETINFO
#define SOCKETINFO

#include <WS2tcpip.h>

#define DATA_BUFSIZE 1024
/** Structure for Completion Routines **/
typedef struct _SOCKET_INFORMATION
{
    SOCKET socket;
    WSABUF datagram;
    DWORD bytesRECV;
    DWORD bytesSEND;
    struct ip_mreq addr;
    SOCKADDR_IN sockAddr;
    OVERLAPPED overlapped;
    WSABUF DataBuf;
} SOCKET_INFORMATION, * LPSOCKET_INFORMATION;

#endif // SOCKETINFO

