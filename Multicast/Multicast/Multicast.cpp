#include "Multicast.h"

int main()
{
    LPSOCKET_INFORMATION lpSocketInfo;
    char multicastAddr[16] = TIMECAST_ADDR;
    struct ip_mreq stMreq;
    WORD wVersionRequested = MAKEWORD (2,2);
    WSAData wsaData;
    int loop;
    
    if ((lpSocketInfo = (LPSOCKET_INFORMATION) GlobalAlloc(GPTR, sizeof(SOCKET_INFORMATION))) == NULL)
    {
       DisplayError("GlobalAlloc failed", GetLastError());
       return -1;
    }

    WSAStartup(wVersionRequested, &wsaData);

    if(InitMulticastSocket(lpSocketInfo) < 0)
    {
        return -1;
    }

    stMreq.imr_multiaddr.s_addr = inet_addr(multicastAddr);
    stMreq.imr_interface.s_addr = INADDR_ANY;
    if(SocketOption(lpSocketInfo->Socket, (char *)&stMreq, sizeof(stMreq)) == SOCKET_ERROR)
    {
        DisplayError("setsockopt IP_ADD_MEMBERSHIP failed", WSAGetLastError());
        return -1;
    }

    loop = 0;
    if(SocketOption(lpSocketInfo->Socket, (char *)&loop, sizeof(loop)) == SOCKET_ERROR)
    {
        DisplayError("setsockopt IP_MULTICAST_LOOP failed", WSAGetLastError());
        return -1;
    }

    return MulticastSendLoop(lpSocketInfo);
}

int MulticastSendLoop(LPSOCKET_INFORMATION lpSocketInfo)
{
    int serverLen;
    int index;
    DWORD flags;
    DWORD sendBytes;
    WSAEVENT eventArray[1];

    strcpy(lpSocketInfo->Buffer, "This is from a Completion Port UDP Server");

    flags = 0;
    serverLen = sizeof(lpSocketInfo->Server);
    
    while(TRUE)
    {
        if (WSASendTo(lpSocketInfo->Socket, &(lpSocketInfo->DataBuf), 1, &sendBytes, flags, (sockaddr *)&(lpSocketInfo->Client), sizeof(lpSocketInfo->Client), &(lpSocketInfo->Overlapped), WorkerRoutine) == SOCKET_ERROR)
        {
            if (WSAGetLastError() != WSA_IO_PENDING)
            {
                DisplayError("WSASendTo failed", WSAGetLastError()); 
                return -1;
            }
        }

        eventArray[0] = WSACreateEvent(); 
        while(TRUE)
        {
            index = WSAWaitForMultipleEvents(1, eventArray, FALSE, WSA_INFINITE, TRUE);

            if (index == WAIT_IO_COMPLETION)
            {
                break;
            }
            else
            {
                DisplayError("WSAWaitForMultipleEvents failed", WSAGetLastError());
                return -1;
            }
        }
    }

    return 0;
}

void CALLBACK WorkerRoutine(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags)
{
    DWORD sendBytes; 
    DWORD flags;
    int clientLen;

    LPSOCKET_INFORMATION SI = (LPSOCKET_INFORMATION) Overlapped;

    if (Error != 0 || BytesTransferred == 0)
    {
        DisplayError("Routine error", Error);
        closesocket(SI->Socket);
        GlobalFree(SI);
        return;
    }

    SI->BytesSEND += BytesTransferred;
    flags = 0;
    ZeroMemory(Overlapped, sizeof(WSAOVERLAPPED));

    SI->DataBuf.len = DATA_BUFSIZE;
    SI->DataBuf.buf = SI->Buffer;

    clientLen = sizeof(SI->Client);

    if (WSASendTo(SI->Socket, &(SI->DataBuf), 1, &sendBytes, flags, (sockaddr *)&(SI->Client), clientLen, &(SI->Overlapped), WorkerRoutine) == SOCKET_ERROR)
    {
        if (WSAGetLastError() != WSA_IO_PENDING)
        {
            DisplayError("WSASendTo failed", WSAGetLastError());
            return;
        }
    }

    printf("\nRoutine: %s\n", SI->Buffer);

    if (WSARecvFrom(SI->Socket, &(SI->DataBuf), 1, &sendBytes, &flags, (sockaddr *)&(SI->Client), &clientLen, &(SI->Overlapped), WorkerRoutine) == SOCKET_ERROR)
    {
        if (WSAGetLastError() != WSA_IO_PENDING)
        {
            DisplayError("WSARecvFrom failed", WSAGetLastError());
            return;
        }
    }
    
    Sleep(TIMECAST_INTRVL * 1000);
}

int InitMulticastSocket(LPSOCKET_INFORMATION lpSocketInfo)
{
    if ((lpSocketInfo->Socket = WSASocket(PF_INET, SOCK_DGRAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
    {
        DisplayError("Socket error", WSAGetLastError());
        return -1;
    }
    
    memset ((char *)&lpSocketInfo->Server, 0, sizeof(lpSocketInfo->Server));
	lpSocketInfo->Server.sin_family = AF_INET;
    lpSocketInfo->Server.sin_port = 0;
	lpSocketInfo->Server.sin_addr.s_addr = htonl(INADDR_ANY);
    
    lpSocketInfo->Client.sin_family      = AF_INET;
    lpSocketInfo->Client.sin_addr.s_addr = inet_addr(TIMECAST_ADDR);
    lpSocketInfo->Client.sin_port        = htons(TIMECAST_PORT);

    if (bind (lpSocketInfo->Socket, (struct sockaddr *)&lpSocketInfo->Server, sizeof(lpSocketInfo->Server)) == -1)
    {
        DisplayError("Bind error", WSAGetLastError());
        return -1;
    }

    ZeroMemory(&(lpSocketInfo->Overlapped), sizeof(WSAOVERLAPPED));
    lpSocketInfo->BytesRECV = 0;
    lpSocketInfo->BytesSEND = 0;
    lpSocketInfo->DataBuf.len = DATA_BUFSIZE;
    lpSocketInfo->DataBuf.buf = lpSocketInfo->Buffer;

    return 0;
}

int SocketOption(SOCKET socket, char *opt, int size)
{
    return setsockopt(socket, IPPROTO_IP, IP_MULTICAST_LOOP, opt, size);
}

void DisplayError(char *errStr, int errCode)
{
    fprintf(stderr, "%s: %d\n", errStr, errCode);
}