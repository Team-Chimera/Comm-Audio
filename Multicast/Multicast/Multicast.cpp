#include "Multicast.h"

int main()
{
    LPSOCKET_INFORMATION lpSocketInfo;
    char multicastAddr[16] = TIMECAST_ADDR;
    struct ip_mreq stMreq;
    WORD wVersionRequested = MAKEWORD (2,2);
    WSAData wsaData;
    int index;
    int loop;
    int serverLen, clientLen;
    DWORD flags;
    DWORD sendBytes;
    WSAEVENT eventArray[1];
    
    WSAStartup(wVersionRequested, &wsaData);

    if ((lpSocketInfo = (LPSOCKET_INFORMATION) GlobalAlloc(GPTR, sizeof(SOCKET_INFORMATION))) == NULL)
    {
       fprintf(stderr, "GlobalAlloc() failed with error %d\n", GetLastError());
       return -1;
    } 

    if ((lpSocketInfo->Socket = WSASocket(PF_INET, SOCK_DGRAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
    {
        fprintf(stderr, "Can't create a socket: %d", WSAGetLastError());
        return -1;
    }
    
    memset ((char *)&lpSocketInfo->Server, 0, sizeof(lpSocketInfo->Server));
	lpSocketInfo->Server.sin_family = AF_INET;
    lpSocketInfo->Server.sin_port = 0;
	lpSocketInfo->Server.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind (lpSocketInfo->Socket, (struct sockaddr *)&lpSocketInfo->Server, sizeof(lpSocketInfo->Server)) == -1)
    {
        fprintf(stderr, "Can't bind name to socket: %d", WSAGetLastError());
        return -1;
    }

    stMreq.imr_multiaddr.s_addr = inet_addr(multicastAddr);
    stMreq.imr_interface.s_addr = INADDR_ANY;
    if(setsockopt(lpSocketInfo->Socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&stMreq, sizeof(stMreq)) == SOCKET_ERROR)
    {
        printf ("setsockopt() IP_ADD_MEMBERSHIP address %s failed, Err: %d\n", multicastAddr, WSAGetLastError());
        return -1;
    }

    /* Disable loopback */
    loop = 0;
    if(setsockopt(lpSocketInfo->Socket, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&loop, sizeof(loop)) == SOCKET_ERROR)
    {
        printf ("setsockopt() IP_MULTICAST_LOOP failed, Err: %d\n", WSAGetLastError());
        return -1;
    }

    /* Assign our destination address */
    lpSocketInfo->Client.sin_family      = AF_INET;
    lpSocketInfo->Client.sin_addr.s_addr = inet_addr(multicastAddr);
    lpSocketInfo->Client.sin_port        = htons(TIMECAST_PORT);

    // Step 3: Now that we have an accepted socket, 
    // start processing I/O using overlapped I/O with a completion routine.  
    // To get the overlapped I/O processing started, 
    // first submit an overlapped WSARecv() request.

    strcpy(lpSocketInfo->Buffer, "This is from a Completion Port UDP Server");

    flags = 0;
    serverLen = sizeof(lpSocketInfo->Server);

    ZeroMemory(&(lpSocketInfo->Overlapped), sizeof(WSAOVERLAPPED));
    lpSocketInfo->BytesRECV = 0;
    lpSocketInfo->BytesSEND = 0;
    lpSocketInfo->DataBuf.len = DATA_BUFSIZE;
    lpSocketInfo->DataBuf.buf = lpSocketInfo->Buffer;
    
    // Step 4: Post an asynchronous WSARecv() request
    // on the socket by specifying the WSAOVERLAPPED structure as a parameter, 
    // and supply the WorkerRoutine function below as the completion routine
    while(TRUE)
    {
        if (WSASendTo(lpSocketInfo->Socket, &(lpSocketInfo->DataBuf), 1, &sendBytes, flags, (sockaddr *)&(lpSocketInfo->Client), sizeof(lpSocketInfo->Client), &(lpSocketInfo->Overlapped), WorkerRoutine) == SOCKET_ERROR)
        {
            if (WSAGetLastError() != WSA_IO_PENDING)
            {
                fprintf(stderr, "WSASendTo() failed with error %d\n", WSAGetLastError()); 
                return -1;
            }
        }

        // Since the WSAWaitForMultipleEvents() API requires waiting on 
        // one or more event objects, we will have to create a dummy 
        // event object.  
        // As an alternative, we can use SleepEx() instead.

        eventArray[0] = WSACreateEvent(); 
        while(TRUE)
        {
            // Step 5: 
            index = WSAWaitForMultipleEvents(1, eventArray, FALSE, WSA_INFINITE, TRUE);

            // Step 6:
            if (index == WAIT_IO_COMPLETION)
            {
                // An overlapped request completion routine just completed.  
                // Continue servicing more completion routines.
                break;
            }
            else
            {
                // A bad error occurred--stop processing!  
                // If we were also processing an event object, 
                // this could be an index to the event array.
                fprintf(stderr, "WSAWaitForMultipleEvents() failed with error %d\n", WSAGetLastError());
                return -1;
            }
        }
    }
}


void CALLBACK WorkerRoutine(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags)
{
    DWORD sendBytes; 
    DWORD flags;
    int clientLen;

    // Reference the WSAOVERLAPPED structure as a SOCKET_INFORMATION structure
    LPSOCKET_INFORMATION SI = (LPSOCKET_INFORMATION) Overlapped;

    if (Error != 0 || BytesTransferred == 0)
    {
        // Either a bad error occurred on the socket or 
        // the socket was closed by a peer 
        closesocket(SI->Socket);
        GlobalFree(SI);
        return;
    }

    // At this point, an overlapped WSARecv() request completed successfully.  
    // Now we can retrieve the received data that is contained in the variable DataBuf.  
    // After processing the received data, we need to post another overlapped 
    // WSARecv() or WSASend() request.  
    // For simplicity, we will post another WSARecv() request.
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
            printf("WSASendTo failed with error %d\n", WSAGetLastError());
            return;
        }
    }

    printf("\nRoutine: %s\n", SI->Buffer);

    if (WSARecvFrom(SI->Socket, &(SI->DataBuf), 1, &sendBytes, &flags, (sockaddr *)&(SI->Client), &clientLen, &(SI->Overlapped), WorkerRoutine) == SOCKET_ERROR)
    {
        if (WSAGetLastError() != WSA_IO_PENDING)
        {
            printf("WSARecvFrom failed with error %d\n", WSAGetLastError());
            return;
        }
    }
    /* Wait for the specified interval */
    Sleep(TIMECAST_INTRVL * 1000);

    printf("\nAfter: %s\n", SI->Buffer);
}