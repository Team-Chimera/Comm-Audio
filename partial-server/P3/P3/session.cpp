#include "stdafx.h"

#include "session.h"

std::map<SOCKET, LPMUSIC_SESSION> SESSIONS;

using namespace std;

bool createSession(SOCKET c, char* a)
{
	LPMUSIC_SESSION m;
	if ((m = (LPMUSIC_SESSION) GlobalAlloc(GPTR,
         sizeof(MUSIC_SESSION))) == NULL)
      {
         printf("music GlobalAlloc() failed with error %d\n", GetLastError());
         return false;
      } 

	if( !( createSocketInfo( &(m->control), c ) && createThreads(m) && createSems(m) ))
	{
		return false;
	}
	// should be ok
	strcpy_s(m->ip, a);

	WaitForSingleObject(sessionsSem, INFINITE);
	SESSIONS.insert(SessionPair(c, m));
	ReleaseSemaphore(sessionsSem, 1, 0);

	//start the threads here
	ResumeThread(m->control_thr);
	ResumeThread(m->mic_rcv_thr);
	ResumeThread(m->mic_send_thr);
	ResumeThread(m->send_thr);

	return true;
}

/*
	probably open a socket for mic both ways, and set up the si for that

	might be better to do them on the fly though?? after a request has been made
*/
bool initSockets(LPMUSIC_SESSION m, SOCKET control)
{
	return createSocketInfo(&(m->control), control);
}

bool createThreads(LPMUSIC_SESSION m)
{
	return createWorkerThread(controlThread, &(m->control_thr), m, CREATE_SUSPENDED) 
		&& createWorkerThread(micSendThread, &(m->mic_send_thr), m, CREATE_SUSPENDED) 
		&& createWorkerThread(micRcvThread, &(m->mic_rcv_thr), m, CREATE_SUSPENDED) 
		&& createWorkerThread(sendFileThread, &(m->send_thr), m, CREATE_SUSPENDED);
}


DWORD WINAPI controlThread(LPVOID lpParameter)
{
	DWORD RecvBytes, result, flags;

	flags = 0;
	LPMUSIC_SESSION m = (LPMUSIC_SESSION) lpParameter;
	//post rcv call waiting for data
	if ( WSARecv( m->control.Socket, &(m->control.DataBuf), 1, &RecvBytes, &flags, &(m->control.Overlapped), controlRoutine ) == SOCKET_ERROR)
      {
         if (WSAGetLastError() != WSA_IO_PENDING)
         {
            printf("WSARecv() failed with error %d\n", WSAGetLastError());
            return FALSE;
         }
      }

	// get into alertable state
	while(1)
	{
		result = SleepEx(INFINITE, TRUE);

         if (result!= WAIT_IO_COMPLETION)
         {
            printf("error on control socket for session %d", m->control.Socket);
            break;
         } 
	}
	
	return FALSE;
}

void CALLBACK controlRoutine(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags)
{
	//do like control in other one, if send make sure all is sent, if rcv, do based on thing recieved action
	// search set of sessions using the control LPSOCKET

	DWORD SendBytes, RecvBytes;
   DWORD Flags;
   string message_rcv;
   LPMUSIC_SESSION m;
   vector<string> incoming;


   // Reference the WSAOVERLAPPED structure as a SOCKET_INFORMATION structure
   LPSOCKET_INFORMATION SI = (LPSOCKET_INFORMATION) Overlapped;
   m = getSession(SI->Socket); // null check...

   if (Error != 0)
   {
     printf("I/O operation failed with error %d\n", Error);
   }

   if (BytesTransferred == 0) // nothing sent or rcvd...
   {
      printf("Closing control socket %d\n", SI->Socket);
   }

   if (Error != 0 || BytesTransferred == 0)
   {
      closesocket(SI->Socket);
      GlobalFree(SI);
	  // closeSession
      return;
   }

   // Check to see if the BytesRECV field equals zero. If this is so, then
   // this means a WSARecv call just completed so update the BytesRECV field
   // with the BytesTransferred value from the completed WSARecv() call.

   if (SI->BytesRECV == 0) // after a rcv
   {
      SI->BytesRECV = BytesTransferred;
      SI->BytesSEND = 0;
	  //rcv = true;
	  message_rcv.assign(SI->Buffer);
	  Split(message_rcv, incoming);

	  if(incoming.empty() || incoming[0].size() != 1) // not a control char
		  incoming.emplace(incoming.begin(), "x"); // so default switch will run
 
	  switch ( incoming.at(0).at(0) ) // first char in first string
	  {
		case 't':
			//call tcp setup function(open socket in tcp, need to know session
			m->mode = 't';
			if( !( incoming.size() > 1 && startSend(m, incoming.at(1)) ) )
				strcpy_s(SI->Buffer, DENY);
			break;
		case 'u':
			m->mode = 'u';
			if( !( incoming.size() > 1 && startSend(m, incoming.at(1)) ) )
				strcpy_s(SI->Buffer, DENY);
			break;
		case 'f':
			// cleanup session
			break;
		default:
			strcpy_s(SI->Buffer, DENY);
			// clean up session... after send happens....
	  }

	   SI->bytesToSend = BytesTransferred; // send back same size, that is what they're expecting
   }
   else // after a send
   {
      SI->BytesSEND += BytesTransferred;
	  SI->bytesToSend -= BytesTransferred;
   }

   if (SI->bytesToSend > 0) // send or continue sending
   {

      // Post another WSASend() request.
      // Since WSASend() is not gauranteed to send all of the bytes requested,
      // continue posting WSASend() calls until all received bytes are sent.

      ZeroMemory(&(SI->Overlapped), sizeof(WSAOVERLAPPED));

      SI->DataBuf.buf = SI->Buffer + SI->BytesSEND;
      SI->DataBuf.len = SI->bytesToSend;

      if (WSASend(SI->Socket, &(SI->DataBuf), 1, &SendBytes, 0,
         &(SI->Overlapped), controlRoutine) == SOCKET_ERROR)
      {
         if (WSAGetLastError() != WSA_IO_PENDING)
         {
            printf("WSASend() failed with error %d\n", WSAGetLastError());
            return;
         }
      }
   }
   else // all was sent, ready for next RECV
   {
      SI->BytesRECV = 0;
	  SI->bytesToSend = 0; // just in case it got negative somehow.....

      // Now that there are no more bytes to send post another WSARecv() request.

      Flags = 0;
      ZeroMemory(&(SI->Overlapped), sizeof(WSAOVERLAPPED));
	  ZeroMemory(&(SI->Buffer), sizeof(DATA_BUFSIZE));

      SI->DataBuf.len = DATA_BUFSIZE;
      SI->DataBuf.buf = SI->Buffer; // should this be zeroed??

      if (WSARecv(SI->Socket, &(SI->DataBuf), 1, &RecvBytes, &Flags,
         &(SI->Overlapped), controlRoutine) == SOCKET_ERROR)
      {
         if (WSAGetLastError() != WSA_IO_PENDING )
         {
            printf("WSARecv() failed with error %d\n", WSAGetLastError());
            return;
         }
      }
   }
}

// should be able to handle tcp and udp, just use sendTo and throw in the ip each time
void CALLBACK sendFileRoutine(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags)
{
	// adjust data pointers
	// close socket when all data has been sent
	// need to alert control when we close the socket


	DWORD SendBytes;
	string message_rcv;
	LPMUSIC_SESSION m;
	sockaddr_in client;

   // Reference the WSAOVERLAPPED structure as a SOCKET_INFORMATION structure
   LPSOCKET_INFORMATION SI = (LPSOCKET_INFORMATION) Overlapped;
   m = getSession(SI->Socket);

   if (Error != 0)
   {
     printf("I/O operation failed with error %d\n", Error);
   }

   if (BytesTransferred == 0) // nothing sent or rcvd...
   {
      printf("Closing control socket %d\n", SI->Socket);
   }

   if (Error != 0 || BytesTransferred == 0)
   {
      closesocket(SI->Socket);
      GlobalFree(SI);
	  // closeSession
      return;
   }

      SI->BytesSEND += BytesTransferred;
	  SI->bytesToSend -= BytesTransferred;

   if (SI->bytesToSend > 0) // send or continue sending
   {
		//memcpy(&client, &(m->ip), sizeof(m->ip));
		//client.sin_port = htons(CLIENT_TCP_PORT);

		getIP_Addr(&client, m->ip, CLIENT_TCP_PORT);

      // Post another WSASend() request.
      // Since WSASend() is not gauranteed to send all of the bytes requested,
      // continue posting WSASend() calls until all received bytes are sent.

      ZeroMemory(&(SI->Overlapped), sizeof(WSAOVERLAPPED));

      SI->DataBuf.buf = SI->Buffer + SI->BytesSEND;
      SI->DataBuf.len = SI->bytesToSend;

      if (WSASendTo(m->send.Socket, &(m->send.DataBuf), 1, &SendBytes, 0, (struct sockaddr *)&client, sizeof(client), &(m->send.Overlapped), sendFileRoutine) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
			printf("WSASend() failed with error %d\n", WSAGetLastError());
			return;
			}
		}
   }
   else // all was sent, close this down
   {
     m->sending = false;

   }
	
}

// needs pointer to file to send probably
DWORD WINAPI sendFileThread(LPVOID lpParameter)
{
	LPMUSIC_SESSION m = (LPMUSIC_SESSION) lpParameter;
	DWORD result, SendBytes;
	sockaddr_in client;
	SOCKET temp;

	//memcpy(&client, &(m->ip), sizeof(m->ip));
	//client.sin_port = htons(CLIENT_TCP_PORT);
	//client.sin_family = AF_INET;
	getIP_Addr(&client, m->ip, CLIENT_TCP_PORT);
	printIP(client);

	while(1)
	{

		//wait for signal from control to initiate a send
		WaitForSingleObject(m->sendSem, INFINITE);
		// should check for error

		m->sending = true;
	
		if(m->mode == 't')
		{
			temp = createTCPSOCKET();
			createSocketInfo(&(m->send), temp);
			if( !connectTCPSOCKET(m->send.Socket, client) )
			{
				printf("error on tcp send connecting for session %d", m->control.Socket);
					return FALSE;
			}
		}
		
		long totalSent = 0;

		FILE *f;
		fopen_s(&f, m->filename, "rb");
		fseek(f, 0, SEEK_END);
		m->filesize = ftell(f);
		fseek(f, 0, SEEK_SET);

		m->fileToSend = (unsigned char*) malloc(m->filesize);
		fread(m->fileToSend, m->filesize, 1, f);
		fclose(f);

		m->send.bytesToSend = m->filesize;
		m->send.BytesSEND = 0;
		m->send.DataBuf.buf = (char*)m->fileToSend;
		m->send.DataBuf.len = m->send.bytesToSend;
		
		//call send function
		if (WSASendTo(m->send.Socket, &(m->send.DataBuf), 1, &SendBytes, 0, (struct sockaddr *)&client, sizeof(client), &(m->send.Overlapped), sendFileRoutine) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
			printf("WSASend() failed with error %d\n", WSAGetLastError());
			return FALSE;
			}
		}

		//get into waiting state
		while(1)
		{
			result = SleepEx(INFINITE, TRUE);

			 if (result!= WAIT_IO_COMPLETION)
			 {
				printf("error on tcp send for session %d", m->control.Socket);
				break;
			 } 

			 if( !m->sending )
			 {
				 deleteSocketInfo(&(m->send));

			 }
		}
	}

	return TRUE;
}

LPMUSIC_SESSION getSession(SOCKET s)
{
	LPMUSIC_SESSION m;
	WaitForSingleObject(sessionsSem, INFINITE);
	for(auto const &it : SESSIONS) 
	{
		// it.first is the first key
		// it.second is the data
		// was &it.second before... i think i would need a ** to music session for that though...
		m = (LPMUSIC_SESSION)it.second;
		if( m->control.Socket == s || m->mic_rcv.Socket == s || m->mic_send.Socket == s || m->send.Socket == s)
		{
			ReleaseSemaphore(sessionsSem, 1, 0);
			return m;	
		}
	}
	ReleaseSemaphore(sessionsSem, 1, 0);
	return nullptr;
}

bool startSend(LPMUSIC_SESSION m, string filename)
{
	//verify file is on the list, if not return false
	//add it to the session
	 strcpy_s( m->filename, filename.c_str() );
	 ReleaseSemaphore(m->sendSem, 1, 0);

	 return true;
}

bool createSems(LPMUSIC_SESSION m)
{
	if( (m->sendSem = CreateSemaphore(NULL, 0, 1, NULL)) == NULL )
	{
		printf("error creating semaphores");
		return false;
	}
	return true;
}

DWORD WINAPI micSendThread(LPVOID lpParameter)
{
	return TRUE;
}
DWORD WINAPI micRcvThread(LPVOID lpParameter)
{
	return TRUE;
}