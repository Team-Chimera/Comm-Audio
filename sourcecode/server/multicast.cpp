#include "multicast.h"
#include "helper.h"

/*******************************************************************
** Function: startMulticastThread
**
** Date: March 12th, 2015
**
** Revisions:
**
**
** Designer: Julian Brandrick
**
** Programmer: Julian Brandrick
**
** Interface:
**			bool startMulticastThread(HANLDE *multicastThread)
**
** Parameters:
**          multicastThread - Pointer to a thread handle
**
** Returns:
**			true on success
**          false if error occurs
**
** Notes:
**  The main function for the multicast session. It initializes the 
**  multicast structure and starts sending in a separate thread.
*******************************************************************/
bool startMulticastThread(HANDLE *multicastThread)
{
    LPMULTICAST_INFORMATION lpMulticastInfo;
    char multicastAddr[16] = TIMECAST_ADDR;
    WORD wVersionRequested = MAKEWORD (2,2);
    WSAData wsaData;
    int loop;

    WSAStartup(wVersionRequested, &wsaData);

    if(lpMulticastInfo = (InitMulticastSocket()) == NULL)
    {
        return false;
    }

    lpMulticastInfo->StMreq.imr_multiaddr.s_addr = inet_addr(multicastAddr);
    lpMulticastInfo->StMreq.imr_interface.s_addr = INADDR_ANY;
    if(setsockopt(socket, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&lpMulticastInfo->StMreq, sizeof(lpMulticastInfo->StMreq)) == SOCKET_ERROR)
    {
        DisplayError("setsockopt IP_ADD_MEMBERSHIP failed", WSAGetLastError());
        return false;
    }

    loop = 0;
    if(SocketOption(lpMulticastInfo->Socket, (char *)&loop, sizeof(loop)) == SOCKET_ERROR)
    {
        DisplayError("setsockopt IP_MULTICAST_LOOP failed", WSAGetLastError());
        return false;
    }

    return createWorkerThread(multicastSendLoop, multicastThread, lpMulticastInfo);
}

/*******************************************************************
** Function: multicastSendLoop
**
** Date: March 12th, 2015
**
** Revisions:
**
**
** Designer: Julian Brandrick
**
** Programmer: Julian Brandrick
**
** Interface:
**			DWORD WINAPI multicastSendLoop(LPVOID lpParam)
**
** Parameters:
**          lpParam - Contains a Multicast structure
**
** Returns:
**			FALSE upon failure
**
** Notes:
**  This is the thread routine for the multicast send loop. It
**  sends data over a multicast channel using a completion routine.
*******************************************************************/
DWORD WINAPI multicastSendLoop(LPVOID lpParam)
{
    LPMULTICAST_INFORMATION lpMulticastInfo = (LPMULTICAST_INFORMATION)lpParam;

    int index;
    DWORD flags;
    DWORD sendBytes;
    WSAEVENT eventArray[1];

    strcpy(lpMulticastInfo->Buffer, "This is from a Completion Port UDP Server");

    flags = 0;
    
    while(TRUE)
    {
        if (WSASendTo(lpMulticastInfo->Socket, &(lpMulticastInfo->DataBuf), 1, &sendBytes, flags, (sockaddr *)&(lpMulticastInfo->Client), sizeof(lpMulticastInfo->Client), &(lpMulticastInfo->Overlapped), multicastRoutine) == SOCKET_ERROR)
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

    return FALSE;
}

/*******************************************************************
** Function: multicastRoutine
**
** Date: March 12th, 2015
**
** Revisions:
**
**
** Designer: Julian Brandrick
**
** Programmer: Julian Brandrick
**
** Interface:
**			void CALLBACK multicastRoutine(DWORD Error, 
**                                         DWORD BytesTransferred, 
**                                         LPWSAOVERLAPPED Overlapped, 
**                                         DWORD InFlags)
**
** Parameters:
**          Error - Contains any errors that occur
**          BytesTransferred - Number of bytes transferred
**          Overlapped - Pointer to an overlapped struct
**          InFlags - Flags used for sending
**
** Returns:
**			FALSE upon failure
**
** Notes:
**  This is the completion routine that handles sending data to the
**  multicast clients.
*******************************************************************/
void CALLBACK multicastRoutine(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags)
{
    DWORD sendBytes;
    DWORD flags;
    int clientLen;

    LPMULTICAST_INFORMATION MI = (LPMULTICAST_INFORMATION) Overlapped;

    if (Error != 0 || BytesTransferred == 0)
    {
        DisplayError("Routine error", Error);
        closesocket(MI->Socket);
        GlobalFree(MI);
        return;
    }

    MI->BytesSent += BytesTransferred;
    flags = 0;
    ZeroMemory(Overlapped, sizeof(WSAOVERLAPPED));

    MI->DataBuf.len = DATA_BUFSIZE;
    MI->DataBuf.buf = MI->Buffer;

    clientLen = sizeof(MI->Client);

    if (WSASendTo(MI->Socket, &(MI->DataBuf), 1, &sendBytes, flags, (sockaddr *)&(MI->Client), clientLen, &(MI->Overlapped), multicastRoutine) == SOCKET_ERROR)
    {
        if (WSAGetLastError() != WSA_IO_PENDING)
        {
            DisplayError("WSASendTo failed", WSAGetLastError());
            return;
        }
    }

    printf("\nRoutine: %s\n", MI->Buffer);

    Sleep(TIMECAST_INTRVL * 1000);
}

/*******************************************************************
** Function: initMulticastSocket
**
** Date: March 12th, 2015
**
** Revisions:
**
**
** Designer: Julian Brandrick
**
** Programmer: Julian Brandrick
**
** Interface:
**			LPMULTICAST_INFORMATION initMulticastSocket()
**
** Returns:
**			- An initialized multicast structure upon success
**          - NULL upon failure
**
** Notes:
**  This function completely initializes a multicast structure.
*******************************************************************/
LPMULTICAST_INFORMATION initMulticastSocket()
{
    LPMuLTICAST_INFORMATION lpMulticastInfo;
    SOCKADDR_IN server;

    if ((lpMulticastInfo = (LPSOCKET_INFORMATION) GlobalAlloc(GPTR, sizeof(SOCKET_INFORMATION))) == NULL)
    {
       DisplayError("GlobalAlloc failed", GetLastError());
       return NULL;
    }

    if ((lpMulticastInfo->Socket = WSASocket(PF_INET, SOCK_DGRAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
    {
        DisplayError("Socket error", WSAGetLastError());
        return NULL;
    }

    memset ((char *)&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = 0;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    
    lpMulticastInfo->Client.sin_family      = AF_INET;
    lpMulticastInfo->Client.sin_addr.s_addr = inet_addr(TIMECAST_ADDR);
    lpMulticastInfo->Client.sin_port        = htons(TIMECAST_PORT);

    if (bind (lpMulticastInfo->Socket, (struct sockaddr *)&server, sizeof(server)) == -1)
    {
        DisplayError("Bind error", WSAGetLastError());
        return NULL;
    }

    ZeroMemory(&(lpMulticastInfo->Overlapped), sizeof(WSAOVERLAPPED));
    lpMulticastInfo->BytesSent = 0;
    lpMulticastInfo->DataBuf.len = DATA_BUFSIZE;
    lpMulticastInfo->DataBuf.buf = lpMulticastInfo->Buffer;

    return lpMulticastInfo;
}

/*******************************************************************
** Function: displayError
**
** Date: March 12th, 2015
**
** Revisions:
**
**
** Designer: Julian Brandrick
**
** Programmer: Julian Brandrick
**
** Interface:
**			void displayError(char *errStr, int errCode)
** 
** Parameters:
**          errStr - Error string to be displayed
**          errCode - Error code that was thrown
**
** Returns:
**			void
**
** Notes:
**  This function prints a formatted error string and code to 
**  stderr.
*******************************************************************/
void displayError(char *errStr, int errCode)
{
    fprintf(stderr, "%s: %d\n", errStr, errCode);
}
