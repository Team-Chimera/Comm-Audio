#include "controlChannel.h"
#include <iostream>

using namespace std;

/** Control socket **/
int listenSocket;
int controlSocket;

//thread handle for accepting clients
HANDLE acceptThread = INVALID_HANDLE_VALUE;

/** Events to listen for **/
WSAEVENT acceptEvent;



//march 15th
//TODO: WSA cleanup and setup
void openControlListener()
{

    struct sockaddr_in server;
    u_long mode = 1;

    //create the TCP socket for listening
    if ((listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
    {
        cerr << "Failed to create the socket";
        return;
    }


    if (ioctlsocket(listenSocket, FIONBIO, &mode) != NO_ERROR)
    {
        cerr << "Unable to set socket into non-blocking";
        return;
    }

    //set the socket structure
    memset((char *)&server, 0, sizeof(struct sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(CONTROL_LISTEN_PORT);

    //bind the socket
    if (bind(listenSocket, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)
    {
        cerr << "Failed to bind";
        return;
    }

    //begin listener
    if (listen(listenSocket, 5))
    {
        cerr << "Listener failed.";
        return;
    }


    //create the accept event
    if ((acceptEvent = WSACreateEvent()) == WSA_INVALID_EVENT)
    {
        cerr << "failed to create the event";
        return;
    }

    //create a thread to accept connections
    DWORD threadId;
    if ((acceptThread = CreateThread(NULL, 0, acceptClients, NULL, 0, &threadId)) == NULL)
    {
        cerr << "Unable to create accept thread";
        return;
    }

    cout << "Control listener now ready";

}


DWORD WINAPI acceptClients(LPVOID arg)
{
    HANDLE events[1] = {acceptEvent};
    DWORD index;

    //loop infinitely waiting for clients
    while(true)
    {
        //wait for accept calls
        while(true)
        {
            //Waiting for an event
            index = WSAWaitForMultipleObjects(1, events, FALSE, INFINITE, TRUE);

            if (index == WSA_WAIT_FAILED)
            {
                cerr << "wait for multiple object failed";
                return -1;
            }

            //accept call received
            if (index != WAIT_TO_COMPLETION)
            {
                break;
            }
        }

        //reset the accept event
        WSAResetEvent(events[index - WSA_WAIT_EVENT_0]);

        createNewClient();
    }

    return 0;
}


int createNewClient()
{

}
