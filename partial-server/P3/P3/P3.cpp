// P3.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "session.h"
#include "p3.h"

#pragma comment(lib, "ws2_32.lib")

using namespace std;

WSAData wsadata;

int main(void)
{
	// do gui things

	initWSA(&wsadata);

	// play with multicast... spawn a thread for it

	// create thread for accept stuff
	createWorkerThread(AcceptThread, &acceptThread, 0);
	while(1);

	cleanUp();
	return 0;
}

DWORD WINAPI AcceptThread(LPVOID lpParameter)
{
	SOCKET Accept, temp;
	sockaddr peer;
    int peer_len;

	if( (sessionsSem = CreateSemaphore(NULL, 1, 1, NULL)) == NULL)
	{
		printf("error creating sessionSem\n");
		return FALSE;
	}

	if(!openListenSocket(&Accept, SERVER_TCP_LISTEN_PORT) )
		return FALSE;
	

		while(TRUE)
	   {
		   peer_len = sizeof(peer);
		   ZeroMemory(&peer, peer_len);

		   temp = accept(Accept, &peer, &peer_len);

		    getpeername(temp, &peer, &peer_len);
			sockaddr_in* s_in = (struct sockaddr_in*)&peer;
			char* temp_addr = inet_ntoa( s_in->sin_addr );

			printf("Socket %d is connected to ", temp);
			printIP(*s_in);

			createSession(temp, temp_addr);
	   }

	return TRUE;
}

void cleanUp()
{
	//call session clean up
	// clean up things for multicast
	// get rid of threads in application layer
}

