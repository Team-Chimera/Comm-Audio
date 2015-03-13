// testbed.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#pragma comment(lib, "ws2_32.lib")

#define SERVER_TCP_LISTEN_PORT 9874
#define CLIENT_TCP_PORT 6678
#define BUFFERSIZE 512

using namespace std;

SOCKET Control, Accept, Receive;
string IP = "127.0.0.1";
string START = "t:bible.txt:";
sockaddr peer;
int peer_len;

bool openTCPSend( SOCKET* s, int port, string ip);
int sendTCPMessage( SOCKET* s, std::string message, int size);
bool openListenSocket(SOCKET* s, int port);

int main(void)
{
	printf("opening a tcp port to send...\n\n");

	if(!openTCPSend(&Control, SERVER_TCP_LISTEN_PORT, IP))
	{
		printf("Error encountered, connection aborted\n");
		return 1;
	}

	printf("sending start message...\n\n");
	sendTCPMessage(&Control, START, BUFFERSIZE);

	// need to set up a socket to listen, then receive the file that gets transferred...
	printf("opening listen socket...\n\n");
	if(!openListenSocket(&Accept, CLIENT_TCP_PORT))
		return 1;

	peer_len = sizeof(peer);
	ZeroMemory(&peer, peer_len);
	printf("waiting to accept a connection...\n\n");
	Receive = accept(Accept, &peer, &peer_len);
	char* buf = new char[BUFFERSIZE+1];
	int n;
	printf("connection accepted, receiving data...\n\n");
	while(1)
	{
		n = recv(Receive, buf, BUFFERSIZE, 0);
		buf[n] = '\0';
		printf("%s", buf);
		if(buf[0] == 0x011)
			break;
	}


	printf("done, press enter to close...\n\n");
	WSACleanup();
	getchar();
	return 0;
}

bool openTCPSend( SOCKET* s, int port, string ip)
{
	WSADATA WSAData;
	if ( WSAStartup( MAKEWORD( 2, 2 ), &WSAData ) != 0 ) //No useable DLL
	{
		printf("WSAStartup DLL not found!\n");
		return false;
	}

	struct sockaddr_in server;
	struct hostent	*hp;
	
	// Create a stream socket
	if ((*s = socket (PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		printf("Can't create socket\n");
		return false;
	}

	memset((char *)&server, 0, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);

	// Initialize and set up the address structure

	if ((hp = gethostbyname(ip.c_str())) == NULL)
	{
		printf("Unknown server address\n");
		return false;
	}

	// Copy the server address
	memcpy((char *)&server.sin_addr, hp->h_addr, hp->h_length);
	
	if (connect (*s, (struct sockaddr *)&server, sizeof(server)) != 0)
	{
		printf("error in connect\n");
		return false;
	}
	return true;
}

int sendTCPMessage( SOCKET* s, std::string message, int size)
{
	int result;
	int totalSent = 0;

	while(message.size() > 0)
	{
		if( (result = send (*s, message.c_str(), size, 0)) == SOCKET_ERROR )
		{
			printf("WSARecv() failed with error %d\n", WSAGetLastError() );
			return -1;
		}
		totalSent += result;
		message.erase( 0, result);
	}
	return totalSent;
}

bool openListenSocket(SOCKET* s, int port)
{
	SOCKADDR_IN InternetAddr;
	int Ret;

	if ((*s = socket (AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) 
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

